#include "Gamepad.h"

#include <algorithm>
#include <cmath>

#pragma comment(lib, "xinput.lib")

namespace Engine {

Vector2 Gamepad::ApplyStickDeadZone(SHORT x, SHORT y, SHORT deadZone) {
	float fx = static_cast<float>(x);
	float fy = static_cast<float>(y);
	float magnitude = std::sqrt(fx * fx + fy * fy);

	if (magnitude <= static_cast<float>(deadZone)) {
		return { 0.0f, 0.0f };
	}

	// デッドゾーンの外側を 0〜1 に引き伸ばす（傾け始めが急に大きくならないようにする）
	constexpr float kMax = 32767.0f;
	float clamped = (std::min)(magnitude, kMax);
	float normalized = (clamped - static_cast<float>(deadZone)) / (kMax - static_cast<float>(deadZone));

	return { fx / magnitude * normalized, fy / magnitude * normalized };
}

void Gamepad::Update(DWORD index) {
	index_ = index;

	// 未接続のパッドは一定間隔でしか確認しない
	if (!connected_ && reconnectCounter_ > 0) {
		--reconnectCounter_;
		return;
	}

	XINPUT_STATE state{};
	DWORD result = XInputGetState(index, &state);

	if (result != ERROR_SUCCESS) {
		// 切断（または未接続）。入力をすべてクリアする
		connected_ = false;
		reconnectCounter_ = kReconnectInterval;
		buttons_ = buttonsPrev_ = 0;
		leftStick_ = rightStick_ = { 0.0f, 0.0f };
		leftTrigger_ = rightTrigger_ = leftTriggerPrev_ = rightTriggerPrev_ = 0.0f;
		return;
	}

	connected_ = true;

	buttonsPrev_ = buttons_;
	leftTriggerPrev_ = leftTrigger_;
	rightTriggerPrev_ = rightTrigger_;

	const XINPUT_GAMEPAD& pad = state.Gamepad;
	buttons_ = pad.wButtons;
	leftStick_ = ApplyStickDeadZone(pad.sThumbLX, pad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	rightStick_ = ApplyStickDeadZone(pad.sThumbRX, pad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	leftTrigger_ = pad.bLeftTrigger / 255.0f;
	rightTrigger_ = pad.bRightTrigger / 255.0f;
}

bool Gamepad::PushButton(GamepadButton button) const {
	return (buttons_ & static_cast<WORD>(button)) != 0;
}

bool Gamepad::TriggerButton(GamepadButton button) const {
	WORD mask = static_cast<WORD>(button);
	return (buttons_ & mask) != 0 && (buttonsPrev_ & mask) == 0;
}

bool Gamepad::ReleaseButton(GamepadButton button) const {
	WORD mask = static_cast<WORD>(button);
	return (buttons_ & mask) == 0 && (buttonsPrev_ & mask) != 0;
}

void Gamepad::SetVibration(float low, float high) {
	if (!connected_) {
		return;
	}
	XINPUT_VIBRATION vibration{};
	vibration.wLeftMotorSpeed = static_cast<WORD>(std::clamp(low, 0.0f, 1.0f) * 65535.0f);
	vibration.wRightMotorSpeed = static_cast<WORD>(std::clamp(high, 0.0f, 1.0f) * 65535.0f);
	XInputSetState(index_, &vibration);
}

} // namespace Engine

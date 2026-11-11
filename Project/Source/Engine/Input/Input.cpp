#include "Input.h"
#include <cassert>
#include <cstring>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Engine {

Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

void Input::Initialize(WinApp* winApp) {
	//借りてきたWinAppのインスタンスを記録
	this->winApp_ = winApp;

	HRESULT result;

	result = DirectInput8Create(
		winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		reinterpret_cast<void**>(directInput_.GetAddressOf()), nullptr
	);
	assert(SUCCEEDED(result));

	// キーボード
	result = directInput_->CreateDevice(GUID_SysKeyboard, keyboard_.GetAddressOf(), NULL);
	assert(SUCCEEDED(result));
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = keyboard_->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));

	// マウス
	result = directInput_->CreateDevice(GUID_SysMouse, mouseDevice_.GetAddressOf(), NULL);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
	);
	assert(SUCCEEDED(result));
}

// Trigger/Release判定に使うため、更新前に前フレームの状態を保存してから最新の状態を取得する
void Input::Update() {
	std::memcpy(keyPrev_, key_, sizeof(key_));
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key_), key_);

	mouseStatePrev_ = mouseState_;
	mouseDevice_->Acquire();
	mouseDevice_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);

	UpdateMousePosition();

	for (int i = 0; i < kMaxGamepads; ++i) {
		gamepads_[i].Update(static_cast<DWORD>(i));
	}
}

void Input::UpdateMousePosition() {
	POINT cursor{};
	RECT clientRect{};
	if (!GetCursorPos(&cursor) ||
		!ScreenToClient(winApp_->GetHwnd(), &cursor) ||
		!GetClientRect(winApp_->GetHwnd(), &clientRect)) {
		mouseInside_ = false;
		return;
	}

	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);
	if (clientWidth <= 0.0f || clientHeight <= 0.0f) {
		mouseInside_ = false;
		return;
	}

	mouseInside_ = cursor.x >= 0 && cursor.y >= 0 &&
		static_cast<float>(cursor.x) < clientWidth && static_cast<float>(cursor.y) < clientHeight;

	// ウィンドウが描画解像度と違うサイズに拡大縮小されていても、描画解像度基準の座標にそろえる
	mousePosition_ = {
		static_cast<float>(cursor.x) * static_cast<float>(WinApp::kClientWidth) / clientWidth,
		static_cast<float>(cursor.y) * static_cast<float>(WinApp::kClientHeight) / clientHeight,
	};
}

const Gamepad& Input::GetGamepad(int index) const {
	assert(index >= 0 && index < kMaxGamepads);
	return gamepads_[index];
}

Gamepad& Input::GetGamepad(int index) {
	assert(index >= 0 && index < kMaxGamepads);
	return gamepads_[index];
}

int Input::GetFirstConnectedGamepadIndex() const {
	for (int i = 0; i < kMaxGamepads; ++i) {
		if (gamepads_[i].IsConnected()) {
			return i;
		}
	}
	return -1;
}

bool Input::PushKey(BYTE keyCode) const {
	return key_[keyCode] != 0;
}

bool Input::UpKey(BYTE keyCode) const {
	return key_[keyCode] == 0;
}

bool Input::TriggerKey(BYTE keyCode) const {
	return key_[keyCode] != 0 && keyPrev_[keyCode] == 0;
}

bool Input::ReleaseKey(BYTE keyCode) const {
	return key_[keyCode] == 0 && keyPrev_[keyCode] != 0;
}

bool Input::PushMouseButton(int button) const {
	return (mouseState_.rgbButtons[button] & 0x80) != 0;
}

bool Input::TriggerMouseButton(int button) const {
	return (mouseState_.rgbButtons[button] & 0x80) != 0 &&
	       (mouseStatePrev_.rgbButtons[button] & 0x80) == 0;
}

void Input::Finalize() {
	// 振動したまま終了しないようにする
	for (int i = 0; i < kMaxGamepads; ++i) {
		gamepads_[i].SetVibration(0.0f, 0.0f);
	}

	mouseDevice_.Reset();
	keyboard_.Reset();
	directInput_.Reset();
}

} // namespace Engine

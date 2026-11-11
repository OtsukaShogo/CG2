#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace Engine {

Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

void Input::Initialize(WinApp* winApp) {
	HRESULT result = DirectInput8Create(
		winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		reinterpret_cast<void**>(directInput_.GetAddressOf()), nullptr
	);
	assert(SUCCEEDED(result));

	keyboard_.Initialize(directInput_.Get(), winApp->GetHwnd());
	mouse_.Initialize(directInput_.Get(), winApp->GetHwnd());
}

void Input::Update() {
	keyboard_.Update();
	mouse_.Update();

	for (int i = 0; i < kMaxGamepads; ++i) {
		gamepads_[i].Update(static_cast<DWORD>(i));
	}
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

void Input::Finalize() {
	// 振動したまま終了しないようにする
	for (int i = 0; i < kMaxGamepads; ++i) {
		gamepads_[i].SetVibration(0.0f, 0.0f);
	}

	mouse_.Finalize();
	keyboard_.Finalize();
	directInput_.Reset();
}

} // namespace Engine

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

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT result;

	result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		reinterpret_cast<void**>(directInput_.GetAddressOf()), nullptr
	);
	assert(SUCCEEDED(result));

	// キーボード
	result = directInput_->CreateDevice(GUID_SysKeyboard, keyboard_.GetAddressOf(), NULL);
	assert(SUCCEEDED(result));
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = keyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));

	// マウス
	result = directInput_->CreateDevice(GUID_SysMouse, mouseDevice_.GetAddressOf(), NULL);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));
	result = mouseDevice_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
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
	mouseDevice_.Reset();
	keyboard_.Reset();
	directInput_.Reset();
}

} // namespace Engine

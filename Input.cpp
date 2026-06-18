#include "Input.h"
#include <cassert>
#include <cstring>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	HRESULT result;

	result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr
	);
	assert(SUCCEEDED(result));

	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	std::memcpy(keyPrev_, key_, sizeof(key_));
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key_), key_);
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

void Input::Finalize() {
	keyboard_->Release();
	keyboard_ = nullptr;
	directInput_->Release();
	directInput_ = nullptr;
}

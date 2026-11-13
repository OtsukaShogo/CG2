#include "Keyboard.h"
#include <cassert>
#include <cstring>

namespace Engine {

void Keyboard::Initialize(IDirectInput8* directInput, HWND hwnd) {
	HRESULT result;

	result = directInput->CreateDevice(GUID_SysKeyboard, device_.GetAddressOf(), NULL);
	assert(SUCCEEDED(result));
	result = device_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	result = device_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

// Trigger/Release判定に使うため、更新前に前フレームの状態を保存してから最新の状態を取得する
void Keyboard::Update() {
	std::memcpy(keyPrev_, key_, sizeof(key_));
	device_->Acquire();
	// 取得に失敗した（ウィンドウが非アクティブ等でデバイスを失った）場合は、押されたままにならないよう全キーを離した状態にする
	if (FAILED(device_->GetDeviceState(sizeof(key_), key_))) {
		std::memset(key_, 0, sizeof(key_));
	}
}

void Keyboard::Finalize() {
	device_.Reset();
}

bool Keyboard::PushKey(BYTE keyCode) const {
	if (blocked_) return false;
	return key_[keyCode] != 0;
}

bool Keyboard::UpKey(BYTE keyCode) const {
	if (blocked_) return true; // 押されていない扱い
	return key_[keyCode] == 0;
}

bool Keyboard::TriggerKey(BYTE keyCode) const {
	if (blocked_) return false;
	return key_[keyCode] != 0 && keyPrev_[keyCode] == 0;
}

bool Keyboard::ReleaseKey(BYTE keyCode) const {
	if (blocked_) return false;
	return key_[keyCode] == 0 && keyPrev_[keyCode] != 0;
}

} // namespace Engine

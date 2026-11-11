#include "Mouse.h"
#include <cassert>

#include "../Core/WinApp.h"

namespace Engine {

void Mouse::Initialize(IDirectInput8* directInput, HWND hwnd) {
	hwnd_ = hwnd;

	HRESULT result;

	result = directInput->CreateDevice(GUID_SysMouse, device_.GetAddressOf(), NULL);
	assert(SUCCEEDED(result));
	result = device_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));
	result = device_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Mouse::Update() {
	statePrev_ = state_;
	device_->Acquire();
	device_->GetDeviceState(sizeof(DIMOUSESTATE), &state_);

	UpdatePosition();
}

void Mouse::UpdatePosition() {
	POINT cursor{};
	RECT clientRect{};
	if (!GetCursorPos(&cursor) ||
		!ScreenToClient(hwnd_, &cursor) ||
		!GetClientRect(hwnd_, &clientRect)) {
		inside_ = false;
		return;
	}

	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);
	if (clientWidth <= 0.0f || clientHeight <= 0.0f) {
		inside_ = false;
		return;
	}

	inside_ = cursor.x >= 0 && cursor.y >= 0 &&
		static_cast<float>(cursor.x) < clientWidth && static_cast<float>(cursor.y) < clientHeight;

	// ウィンドウが描画解像度と違うサイズに拡大縮小されていても、描画解像度基準の座標にそろえる
	position_ = {
		static_cast<float>(cursor.x) * static_cast<float>(WinApp::kClientWidth) / clientWidth,
		static_cast<float>(cursor.y) * static_cast<float>(WinApp::kClientHeight) / clientHeight,
	};
}

void Mouse::Finalize() {
	device_.Reset();
}

bool Mouse::PushButton(MouseButton button) const {
	return (state_.rgbButtons[static_cast<int>(button)] & 0x80) != 0;
}

bool Mouse::TriggerButton(MouseButton button) const {
	int i = static_cast<int>(button);
	return (state_.rgbButtons[i] & 0x80) != 0 && (statePrev_.rgbButtons[i] & 0x80) == 0;
}

bool Mouse::ReleaseButton(MouseButton button) const {
	int i = static_cast<int>(button);
	return (state_.rgbButtons[i] & 0x80) == 0 && (statePrev_.rgbButtons[i] & 0x80) != 0;
}

} // namespace Engine

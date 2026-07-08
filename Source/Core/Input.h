#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>

class Input {
public:
	static Input* GetInstance();

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();
	void Finalize();

	// キーボード
	bool PushKey(BYTE keyCode) const;
	bool UpKey(BYTE keyCode) const;
	bool TriggerKey(BYTE keyCode) const;
	bool ReleaseKey(BYTE keyCode) const;

	// マウス（今フレームの相対移動量）
	long GetMouseDeltaX() const { return mouseState_.lX; }
	long GetMouseDeltaY() const { return mouseState_.lY; }
	long GetMouseDeltaWheel() const { return mouseState_.lZ; }

	// マウスボタン (0=左, 1=右, 2=中)
	bool PushMouseButton(int button) const;
	bool TriggerMouseButton(int button) const;

private:
	Input() = default;
	~Input() = default;

	IDirectInput8* directInput_ = nullptr;
	IDirectInputDevice8* keyboard_ = nullptr;
	IDirectInputDevice8* mouseDevice_ = nullptr;
	BYTE key_[256] = {};
	BYTE keyPrev_[256] = {};
	DIMOUSESTATE mouseStatePrev_ = {};
	DIMOUSESTATE mouseState_ = {};
};

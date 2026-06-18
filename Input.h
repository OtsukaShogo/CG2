#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>

class Input {
public:
	static Input* GetInstance();

	// コピー禁止
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	// DirectInput初期化・キーボードデバイス生成
	void Initialize(HINSTANCE hInstance, HWND hwnd);

	// キー状態を更新（毎フレーム呼ぶ）
	void Update();

	// 終了処理
	void Finalize();

	// キーを押した状態か
	bool PushKey(BYTE keyCode) const;

	// キーを離した状態か
	bool UpKey(BYTE keyCode) const;

	// キーを押した瞬間か
	bool TriggerKey(BYTE keyCode) const;

	// キーを離した瞬間か
	bool ReleaseKey(BYTE keyCode) const;

private:
	Input() = default;
	~Input() = default;

private:
	IDirectInput8* directInput_ = nullptr;
	IDirectInputDevice8* keyboard_ = nullptr;
	BYTE key_[256] = {};
	BYTE keyPrev_[256] = {};
};

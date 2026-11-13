#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <wrl/client.h>

#include "../Core/WinApp.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Gamepad.h"

namespace Engine {

/// <summary>
/// キーボード・マウス・ゲームパッドをまとめて管理する入力のまとめ役。
/// 各デバイスの入力は GetKeyboard() / GetMouse() / GetGamepad() から取得する
/// </summary>
class Input {
public:
	/// <summary>
	/// 同時に扱えるゲームパッドの最大数
	/// </summary>
	static constexpr int kMaxGamepads = XUSER_MAX_COUNT;

	/// <summary>
	/// シングルトンインスタンスを取得する
	/// </summary>
	/// <returns>Inputのインスタンス</returns>
	[[nodiscard]] static Input* GetInstance();

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	/// <summary>
	/// DirectInputと各入力デバイスを初期化する
	/// </summary>
	/// <param name="winApp">対象のウィンドウを持つWinApp</param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// すべての入力デバイスの状態を最新のものに更新する
	/// </summary>
	void Update();

	/// <summary>
	/// 終了処理を行う
	/// </summary>
	void Finalize();

	/// <summary>
	/// キーボードを取得する
	/// </summary>
	[[nodiscard]] const Keyboard& GetKeyboard() const { return keyboard_; }

	/// <summary>
	/// マウスを取得する
	/// </summary>
	[[nodiscard]] const Mouse& GetMouse() const { return mouse_; }

	/// <summary>
	/// ゲームパッドを取得する。未接続でも取得でき、その場合は入力がすべて無効（IsConnected()がfalse）になる
	/// </summary>
	/// <param name="index">パッド番号（0〜3）</param>
	[[nodiscard]] const Gamepad& GetGamepad(int index = 0) const;

	/// <summary>
	/// ゲームパッドを取得する（振動の設定など、状態を変更する操作用）
	/// </summary>
	/// <param name="index">パッド番号（0〜3）</param>
	[[nodiscard]] Gamepad& GetGamepad(int index = 0);

	/// <summary>
	/// 接続されているゲームパッドの最初の番号を取得する（なければ-1）。1人用ゲームで接続中のパッドを自動選択するのに使う
	/// </summary>
	/// <returns>パッド番号または-1</returns>
	[[nodiscard]] int GetFirstConnectedGamepadIndex() const;

	/// <summary>
	/// ウィンドウがアクティブ（フォーカスがある）かを取得する。非アクティブの間、キーボード・マウス・ゲームパッドの入力は無効になる
	/// </summary>
	[[nodiscard]] bool IsWindowActive() const { return windowActive_; }

private:
	Input() = default;
	~Input() = default;

private:
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
	Keyboard keyboard_;
	Mouse mouse_;
	HWND hwnd_ = nullptr;
	bool windowActive_ = true;
	Gamepad gamepads_[kMaxGamepads];
};

} // namespace Engine

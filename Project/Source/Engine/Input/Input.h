#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <wrl/client.h>
#include"../Core/WinApp.h"
#include "../Math/Vector2.h"
#include "Gamepad.h"

namespace Engine {

/// <summary>
/// DirectInputを使ったキーボード・マウス入力を管理するクラス
/// </summary>
class Input {
public:
	//namespace省略
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	/// <summary>
	/// シングルトンインスタンスを取得する
	/// </summary>
	/// <returns>Inputのインスタンス</returns>
	[[nodiscard]] static Input* GetInstance();

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	/// <summary>
	/// DirectInputとキーボード・マウスデバイスを初期化する
	/// </summary>
	/// <param name="hInstance">アプリケーションのインスタンスハンドル</param>
	/// <param name="hwnd">対象のウィンドウハンドル</param>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// キーボード・マウスの状態を最新のものに更新する
	/// </summary>
	void Update();

	/// <summary>
	/// 終了処理を行う
	/// </summary>
	void Finalize();

	/// <summary>
	/// 指定キーが押されているかを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード</param>
	/// <returns>押されている場合はtrue</returns>
	bool PushKey(BYTE keyCode) const;

	/// <summary>
	/// 指定キーが離されているかを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード</param>
	/// <returns>離されている場合はtrue</returns>
	bool UpKey(BYTE keyCode) const;

	/// <summary>
	/// 指定キーが今フレームで押された瞬間かを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード</param>
	/// <returns>今フレームで押された場合はtrue</returns>
	bool TriggerKey(BYTE keyCode) const;

	/// <summary>
	/// 指定キーが今フレームで離された瞬間かを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード</param>
	/// <returns>今フレームで離された場合はtrue</returns>
	bool ReleaseKey(BYTE keyCode) const;

	/// <summary>
	/// 今フレームのマウスX軸相対移動量を取得する
	/// </summary>
	/// <returns>マウスX軸相対移動量</returns>
	long GetMouseDeltaX() const { return mouseState_.lX; }

	/// <summary>
	/// 今フレームのマウスY軸相対移動量を取得する
	/// </summary>
	/// <returns>マウスY軸相対移動量</returns>
	long GetMouseDeltaY() const { return mouseState_.lY; }

	/// <summary>
	/// 今フレームのマウスホイール移動量を取得する
	/// </summary>
	/// <returns>マウスホイール移動量</returns>
	long GetMouseDeltaWheel() const { return mouseState_.lZ; }

	/// <summary>
	/// 指定のマウスボタンが押されているかを取得する（0=左, 1=右, 2=中）
	/// </summary>
	/// <param name="button">判定するマウスボタン番号</param>
	/// <returns>押されている場合はtrue</returns>
	bool PushMouseButton(int button) const;

	/// <summary>
	/// 指定のマウスボタンが今フレームで押された瞬間かを取得する（0=左, 1=右, 2=中）
	/// </summary>
	/// <param name="button">判定するマウスボタン番号</param>
	/// <returns>今フレームで押された場合はtrue</returns>
	bool TriggerMouseButton(int button) const;

	/// <summary>
	/// マウスカーソルの位置をスクリーン座標（クライアント領域の左上原点、描画解像度基準のピクセル）で取得する。
	/// ウィンドウが拡大縮小されていても、描画解像度（WinApp::kClientWidth/Height）に合わせた値になる
	/// </summary>
	/// <returns>マウスカーソルのスクリーン座標</returns>
	[[nodiscard]] Vector2 GetMousePosition() const { return mousePosition_; }

	/// <summary>
	/// マウスカーソルがウィンドウのクライアント領域の内側にあるかを取得する
	/// </summary>
	/// <returns>内側にある場合はtrue</returns>
	[[nodiscard]] bool IsMouseInsideWindow() const { return mouseInside_; }

	/// <summary>
	/// ゲームパッドを取得する。未接続でも取得でき、その場合は入力がすべて無効（IsConnected()がfalse）になる
	/// </summary>
	/// <param name="index">パッド番号（0〜3）</param>
	/// <returns>ゲームパッド</returns>
	[[nodiscard]] const Gamepad& GetGamepad(int index = 0) const;

	/// <summary>
	/// ゲームパッドを取得する（振動の設定など、状態を変更する操作用）
	/// </summary>
	/// <param name="index">パッド番号（0〜3）</param>
	/// <returns>ゲームパッド</returns>
	[[nodiscard]] Gamepad& GetGamepad(int index = 0);

	/// <summary>
	/// 接続されているゲームパッドの最初の番号を取得する（なければ-1）。1人用ゲームで接続中のパッドを自動選択するのに使う
	/// </summary>
	/// <returns>パッド番号または-1</returns>
	[[nodiscard]] int GetFirstConnectedGamepadIndex() const;

	/// <summary>
	/// 同時に扱えるゲームパッドの最大数
	/// </summary>
	static constexpr int kMaxGamepads = XUSER_MAX_COUNT;

private:
	Input() = default;
	~Input() = default;

	/// <summary>
	/// マウスカーソル位置を取得し、描画解像度基準のスクリーン座標へ変換する
	/// </summary>
	void UpdateMousePosition();

	ComPtr<IDirectInput8> directInput_;
	ComPtr<IDirectInputDevice8> keyboard_;
	ComPtr<IDirectInputDevice8> mouseDevice_;
	BYTE key_[256] = {};
	BYTE keyPrev_[256] = {};
	DIMOUSESTATE mouseStatePrev_ = {};
	DIMOUSESTATE mouseState_ = {};

	Vector2 mousePosition_ = {};
	bool mouseInside_ = false;

	Gamepad gamepads_[kMaxGamepads];

	WinApp* winApp_ = nullptr;
};

} // namespace Engine

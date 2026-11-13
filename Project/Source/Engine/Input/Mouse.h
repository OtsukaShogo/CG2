#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <wrl/client.h>

#include "../Math/Vector2.h"

namespace Engine {

/// <summary>
/// マウスのボタン
/// </summary>
enum class MouseButton : int {
	kLeft = 0,
	kRight = 1,
	kMiddle = 2,
};

/// <summary>
/// DirectInputを使ったマウス入力（ボタン・移動量・ホイール）と、カーソル位置を管理するクラス
/// </summary>
class Mouse {
public:
	/// <summary>
	/// マウスデバイスを生成して初期化する
	/// </summary>
	/// <param name="directInput">DirectInputオブジェクト（Inputが所有）</param>
	/// <param name="hwnd">対象のウィンドウハンドル（カーソル位置の変換にも使う）</param>
	void Initialize(IDirectInput8* directInput, HWND hwnd);

	/// <summary>
	/// 状態を最新のものに更新する（前フレームの状態を保存してから取得する）
	/// </summary>
	void Update();

	/// <summary>
	/// デバイスを解放する
	/// </summary>
	void Finalize();

	/// <summary>
	/// ブロック状態を設定する。trueの間は、ボタンは押されていない・移動量とホイールは0として扱う（ImGuiが入力を使っているときなど）。
	/// カーソル位置は変わらず取得できる
	/// </summary>
	void SetBlocked(bool blocked) { blocked_ = blocked; }

	/// <summary>
	/// 指定のボタンが押されているかを取得する
	/// </summary>
	[[nodiscard]] bool PushButton(MouseButton button) const;

	/// <summary>
	/// 指定のボタンが今フレームで押された瞬間かを取得する
	/// </summary>
	[[nodiscard]] bool TriggerButton(MouseButton button) const;

	/// <summary>
	/// 指定のボタンが今フレームで離された瞬間かを取得する
	/// </summary>
	[[nodiscard]] bool ReleaseButton(MouseButton button) const;

	/// <summary>
	/// 今フレームのX軸相対移動量を取得する
	/// </summary>
	[[nodiscard]] long GetDeltaX() const { return blocked_ ? 0 : state_.lX; }

	/// <summary>
	/// 今フレームのY軸相対移動量を取得する
	/// </summary>
	[[nodiscard]] long GetDeltaY() const { return blocked_ ? 0 : state_.lY; }

	/// <summary>
	/// 今フレームのホイール移動量を取得する
	/// </summary>
	[[nodiscard]] long GetDeltaWheel() const { return blocked_ ? 0 : state_.lZ; }

	/// <summary>
	/// カーソルの位置をスクリーン座標（クライアント領域の左上原点、描画解像度基準のピクセル）で取得する。
	/// ウィンドウが拡大縮小されていても、描画解像度（WinApp::kClientWidth/Height）に合わせた値になる
	/// </summary>
	[[nodiscard]] Vector2 GetPosition() const { return position_; }

	/// <summary>
	/// カーソルがウィンドウのクライアント領域の内側にあるかを取得する
	/// </summary>
	[[nodiscard]] bool IsInsideWindow() const { return inside_; }

private:
	/// <summary>
	/// カーソル位置を取得し、描画解像度基準のスクリーン座標へ変換する
	/// </summary>
	void UpdatePosition();

private:
	Microsoft::WRL::ComPtr<IDirectInputDevice8> device_;
	DIMOUSESTATE state_ = {};
	DIMOUSESTATE statePrev_ = {};

	HWND hwnd_ = nullptr;
	Vector2 position_ = {};
	bool inside_ = false;
	bool blocked_ = false;
};

} // namespace Engine

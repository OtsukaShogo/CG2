#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <wrl/client.h>

namespace Engine {

/// <summary>
/// DirectInputを使ったキーボード・マウス入力を管理するクラス
/// </summary>
class Input {
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
	void Initialize(HINSTANCE hInstance, HWND hwnd);

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

private:
	Input() = default;
	~Input() = default;

	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouseDevice_;
	BYTE key_[256] = {};
	BYTE keyPrev_[256] = {};
	DIMOUSESTATE mouseStatePrev_ = {};
	DIMOUSESTATE mouseState_ = {};
};

} // namespace Engine

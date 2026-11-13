#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>
#include <wrl/client.h>

namespace Engine {

/// <summary>
/// DirectInputを使ったキーボード入力を管理するクラス。キーコードは DIK_～ を使う
/// </summary>
class Keyboard {
public:
	/// <summary>
	/// キーボードデバイスを生成して初期化する
	/// </summary>
	/// <param name="directInput">DirectInputオブジェクト（Inputが所有）</param>
	/// <param name="hwnd">対象のウィンドウハンドル</param>
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
	/// ブロック状態を設定する。trueの間は、すべてのキーが押されていないものとして扱う（ImGuiが入力を使っているときなど）。
	/// 内部の状態更新は続くので、解除した時に誤ったトリガーは発生しない
	/// </summary>
	void SetBlocked(bool blocked) { blocked_ = blocked; }

	/// <summary>
	/// 指定キーが押されているかを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード（DIK_～）</param>
	[[nodiscard]] bool PushKey(BYTE keyCode) const;

	/// <summary>
	/// 指定キーが離されているかを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード（DIK_～）</param>
	[[nodiscard]] bool UpKey(BYTE keyCode) const;

	/// <summary>
	/// 指定キーが今フレームで押された瞬間かを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード（DIK_～）</param>
	[[nodiscard]] bool TriggerKey(BYTE keyCode) const;

	/// <summary>
	/// 指定キーが今フレームで離された瞬間かを取得する
	/// </summary>
	/// <param name="keyCode">判定するキーコード（DIK_～）</param>
	[[nodiscard]] bool ReleaseKey(BYTE keyCode) const;

private:
	Microsoft::WRL::ComPtr<IDirectInputDevice8> device_;
	BYTE key_[256] = {};
	BYTE keyPrev_[256] = {};
	bool blocked_ = false;
};

} // namespace Engine

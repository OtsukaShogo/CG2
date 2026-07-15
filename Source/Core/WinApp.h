#pragma once
#include<cstdint>
#include <Windows.h>

namespace Engine {

/// <summary>
/// Windowsのウィンドウ生成・メッセージ処理を管理するクラス
/// </summary>
class WinApp
{
public:

	/// <summary>
	/// シングルトンインスタンスを取得する
	/// </summary>
	/// <returns>WinAppのインスタンス</returns>
	[[nodiscard]] static WinApp* GetInstance();

	//コピー禁止
	WinApp(const WinApp&) = delete;
	WinApp& operator=(const WinApp&) = delete;

	/// <summary>
	/// ウィンドウを生成し表示する
	/// </summary>
	void Initialize();

	/// <summary>
	/// OSのメッセージを1件処理する。終了メッセージを受け取った場合はfalseを返す
	/// </summary>
	/// <returns>継続する場合はtrue、終了メッセージを受け取った場合はfalse</returns>
	bool ProcessMessage();

	// クライアント領域のサイズ
	static constexpr int32_t kClientWidth = 1280;
	static constexpr int32_t kClientHeight = 720;

	/// <summary>
	/// ウィンドウハンドルを取得する
	/// </summary>
	/// <returns>ウィンドウハンドル</returns>
	const HWND& GetHwnd() const { return hwnd_; }

	/// <summary>
	/// インスタンスハンドルを取得する
	/// </summary>
	/// <returns>インスタンスハンドル</returns>
	HINSTANCE GetHInstance() const { return hInstance_; }

private:

	WinApp() = default;
	~WinApp() = default;

	HWND hwnd_ = nullptr;
	HINSTANCE hInstance_ = nullptr;
};

} // namespace Engine

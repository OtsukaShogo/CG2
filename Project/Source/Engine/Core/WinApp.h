#pragma once
#include<cstdint>
#include <Windows.h>

namespace Engine {

/// <summary>
/// Windowsのウィンドウ生成・メッセージ処理を管理するクラス
/// </summary>
class WinApp
{
public://静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

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
	/// COMを初期化し、ウィンドウを生成し表示する
	/// </summary>
	void Initialize();

	/// <summary>
	/// OSのメッセージを1件処理する。終了メッセージを受け取った場合はtrueを返す
	/// </summary>
	/// <returns>終了メッセージを受け取った場合はtrue、継続する場合はfalse</returns>
	bool ProcessMessage();

	/// <summary>
	/// ウィンドウの破棄、ウィンドウクラスの登録解除、COMの終了処理を行う
	/// </summary>
	void Finalize();

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

	// ウィンドウクラス名（登録と登録解除で同じ名前を使う）
	static constexpr const wchar_t* kWindowClassName = L"CG2WindowClass";

	HWND hwnd_ = nullptr;
	HINSTANCE hInstance_ = nullptr;
};

} // namespace Engine

#include "WinApp.h"

#ifdef USE_IMGUI
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_win32.h"
#endif

#include"DebugUtil.h"

#include <timeapi.h>

#pragma comment(lib,"winmm.lib")

#ifdef USE_IMGUI

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif

namespace Engine {

WinApp* WinApp::GetInstance() {
	static WinApp instance;
	return &instance;
}

/// <summary>
/// ウィンドウメッセージを処理するウィンドウプロシージャ
/// </summary>
/// <param name="hwnd">メッセージの送信先ウィンドウハンドル</param>
/// <param name="msg">メッセージの種類</param>
/// <param name="wparam">メッセージの追加情報</param>
/// <param name="lparam">メッセージの追加情報</param>
/// <returns>メッセージ処理結果</returns>
LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

#endif

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize() {

	// COM初期化（Finalize の CoUninitialize と対応）
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	(void)hr;

	//ウィンドウクラスの設定
	WNDCLASS wc{};

	// インスタンスハンドル
	hInstance_ = GetModuleHandle(nullptr);

	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wc.lpszClassName = kWindowClassName;

	// インスタンスハンドル
	wc.hInstance = hInstance_;

	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	hwnd_ = CreateWindow(
		wc.lpszClassName,        // 利用するクラス名
		L"CG2",                  // タイトルバーの文字 (何でも良い)
		WS_OVERLAPPEDWINDOW,     // よく見るウィンドウスタイル
		CW_USEDEFAULT,           // 表示X座標 (WindowsOSに任せる)
		CW_USEDEFAULT,           // 表示Y座標 (WindowsOSに任せる)
		wrc.right - wrc.left,    // ウィンドウ横幅
		wrc.bottom - wrc.top,    // ウィンドウ縦幅
		nullptr,                 // 親ウィンドウハンドル
		nullptr,                 // メニューハンドル
		wc.hInstance,            // インスタンスハンドル
		nullptr                  // オプション
	);

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);

	DebugUtil::Log("ShowWindow called\n");

	//システムタイマーの分解能を上げる
	timeBeginPeriod(1);
}

bool WinApp::ProcessMessage() {
	MSG msg{};
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return true;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return false;
}

void WinApp::Finalize() {
	// ×ボタンで閉じた場合は WM_DESTROY の時点で破棄済みなので、まだ存在するときだけ破棄する
	if (hwnd_ && IsWindow(hwnd_)) {
		DestroyWindow(hwnd_);
	}
	hwnd_ = nullptr;

	// ウィンドウを破棄した後でないと登録解除できない
	UnregisterClass(kWindowClassName, hInstance_);

	CoUninitialize();
}

} // namespace Engine

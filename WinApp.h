#pragma once
#include<cstdint>
#include <Windows.h>

class WinApp
{
public:

	static WinApp* GetInstance();

	//コピー禁止
	WinApp(const WinApp&) = delete;
	WinApp& operator=(const WinApp&) = delete;

	void Initialize();

	bool ProcessMessage();

	// クライアント領域のサイズ
	static constexpr int32_t kClientWidth = 1280;
	static constexpr int32_t kClientHeight = 720;

	const HWND& GetHwnd() const { return hwnd_; }

private:

	WinApp() = default;
	~WinApp() = default;

	HWND hwnd_ = nullptr;
};
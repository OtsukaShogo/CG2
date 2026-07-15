#include "ImGuiManager.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"

namespace Engine {

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

// ImGui本体・Win32・DX12それぞれのバックエンドを初期化する
void ImGuiManager::Initialize(HWND hwnd, ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {
	if (initialized_) return;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);

	ImGui_ImplDX12_Init(
		device,
		2, // フレーム数（DirectXCommon のバックバッファ数と合わせる）
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvHeap,
		srvHeap->GetCPUDescriptorHandleForHeapStart(),
		srvHeap->GetGPUDescriptorHandleForHeapStart()
	);

	initialized_ = true;
}

void ImGuiManager::BeginFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::EndFrame(ID3D12GraphicsCommandList* commandList) {
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::Finalize() {
	if (!initialized_) return;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	initialized_ = false;
}

} // namespace Engine
#endif

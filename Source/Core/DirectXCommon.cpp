#include "DirectXCommon.h"
#include"D3D12Util.h"

#include<cassert>
#include<thread>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace Engine {

	void DirectXCommon::Initialize(HWND hwnd, uint32_t width, uint32_t height) {
		//FPS固定初期化
		InitializeFixFPS();

		device_ = std::make_unique<D3D12Device>();
		device_->Initialize();

		commandContext_ = std::make_unique<CommandContext>();
		commandContext_->Initialize(device_->Get());

		frameContext_ = std::make_shared<FrameContext>();
		frameContext_->commandList = commandContext_->GetCommandList();

		swapChain_ = std::make_unique<SwapChain>();
		swapChain_->Initialize(device_->GetFactory(), device_->Get(), commandContext_->GetCommandQueue(), hwnd, width, height);

		// SRVヒープ / 深度ステンシル
		InitializeHeapsAndDepthStencil(width, height);

		frameSync_ = std::make_unique<FrameSync>();
		frameSync_->Initialize(device_->Get());

		// ビューポート / シザー
		viewport_.Width = static_cast<float>(width);
		viewport_.Height = static_cast<float>(height);
		viewport_.TopLeftX = 0.0f;
		viewport_.TopLeftY = 0.0f;
		viewport_.MinDepth = 0.0f;
		viewport_.MaxDepth = 1.0f;

		scissorRect_.left = 0;
		scissorRect_.right = static_cast<LONG>(width);
		scissorRect_.top = 0;
		scissorRect_.bottom = static_cast<LONG>(height);
	}

	void DirectXCommon::InitializeHeapsAndDepthStencil(uint32_t width, uint32_t height) {
		ID3D12Device* device = device_->Get();

		srvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
		dsvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

		descriptorSizeSRV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		descriptorSizeRTV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		descriptorSizeDSV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		depthStencilResource_ = CreateDepthStencilTextureResource(device, width, height);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		device->CreateDepthStencilView(
			depthStencilResource_.Get(),
			&dsvDesc,
			dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
	}

	void DirectXCommon::InitializeFixFPS()
	{
		reference_ = std::chrono::steady_clock::now();
	}

	void DirectXCommon::UpdateFixFPS()
	{
		//1/60秒ぴったりの時間
		const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
		//1/60秒よりわずかに短い時間
		const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

		//現在時間を取得する
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		//前回記録からの経過時間を取得する
		std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

		//1/60秒(よりわずかに短い時間)経ってない場合
		if (elapsed < kMinCheckTime) {
			//1/60秒経過するまで微小なスリープを繰り返す
			while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
				//1マイクロ秒スリープ
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			}
		}

		//現在の時間を記録する
		reference_ = std::chrono::steady_clock::now();
	}

	void DirectXCommon::BeginFrame() {
		swapChain_->UpdateBackBufferIndex();

		commandContext_->Reset();

		ID3D12GraphicsCommandList* commandList = commandContext_->GetCommandList();

		// 今フレームで使うコマンドリストをFrameContextへ反映する
		// （将来コマンドリストをフレームごとに切り替える設計になっても、ここの更新だけで済む）
		frameContext_->commandList = commandList;

		// バリア: Present → RenderTarget
		swapChain_->TransitionToRenderTarget(commandList);

		// RTV / DSV
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain_->GetCurrentRTVHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

		// クリア
		float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f, 0, 0, nullptr);

		// OMセット
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// ビューポート / シザー
		commandList->RSSetViewports(1, &viewport_);
		commandList->RSSetScissorRects(1, &scissorRect_);
	}

	void DirectXCommon::EndFrame() {
		ID3D12GraphicsCommandList* commandList = commandContext_->GetCommandList();

		// バリア: RenderTarget → Present
		swapChain_->TransitionToPresent(commandList);

		// コマンドリストを閉じて実行
		commandContext_->CloseAndExecute();

		// フェンスシグナル & 待機
		// 次フレームでコマンドアロケータ・リストを安全にリセットできるようにする
		frameSync_->SignalAndWait(commandContext_->GetCommandQueue());

		// Present
		swapChain_->Present();

		//FPS固定
		UpdateFixFPS();
	}

	// 記録済みのコマンドを即座にGPUへ投入し、完了するまで待つ（初期化処理など、次の描画フレームを待たずに結果が必要な場合に使う）
	void DirectXCommon::FlushCommands() {
		commandContext_->CloseAndExecute();
		frameSync_->SignalAndWait(commandContext_->GetCommandQueue());
		// リセットは次の BeginFrame() に委ねる（Close 済みなので安全）
	}

	void DirectXCommon::Finalize() {
		// Present後にDXGIが投入したGPU作業（バックバッファのcomposite等）も含めて完了を待つ
		frameSync_->SignalAndWait(commandContext_->GetCommandQueue());
		frameSync_->Finalize();

		// コマンドリスト解放後にダングリングポインタが残らないようにする
		frameContext_->commandList = nullptr;

		// シングルトンの ComPtr は main() 返却後まで破棄されないため、明示的に解放してからリークチェックを行う
		depthStencilResource_.Reset();
		dsvDescriptorHeap_.Reset();
		srvDescriptorHeap_.Reset();
		swapChain_->Finalize();
		commandContext_->Finalize();
		device_->Finalize();
	}

} // namespace Engine

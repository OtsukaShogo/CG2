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

		// 描画ごとの定数バッファ（WVP・マテリアル）を切り出すアロケータ
		constantBufferAllocator_ = std::make_unique<ConstantBufferAllocator>();
		constantBufferAllocator_->Initialize(device_->Get(), kFrameCount);
		frameContext_->constantBuffers = constantBufferAllocator_.get();

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

		srvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kSrvHeapSize, true);
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
		// 高精度タイマー（1ms未満の精度で待てる）。使えない環境では nullptr のまま、スピンのみで待つ
		limiterTimer_ = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	}

	void DirectXCommon::UpdateFixFPS()
	{
		using Clock = std::chrono::steady_clock;
		constexpr std::chrono::microseconds kFrameTime(1000000 / 60);   // 1フレームの目標時間
		constexpr std::chrono::microseconds kSpinMargin(1500);          // 目標の直前はタイマーの誤差を避けるためスピンで待つ

		// 次フレームの目標時刻（前回の目標から積み上げるので、待機の誤差が累積しない）
		Clock::time_point target = reference_ + kFrameTime;

		Clock::time_point now = Clock::now();
		if (limiterTimer_ && target - now > kSpinMargin) {
			// 目標の少し手前までは、CPUを使わずにタイマーで寝る
			LARGE_INTEGER dueTime;
			dueTime.QuadPart = -std::chrono::duration_cast<std::chrono::nanoseconds>(target - kSpinMargin - now).count() / 100; // 100ns単位・負の値=相対時間
			SetWaitableTimer(limiterTimer_, &dueTime, 0, nullptr, nullptr, FALSE);
			WaitForSingleObject(limiterTimer_, INFINITE);
		}
		// 残りの短い時間だけスピンで待つ
		while (Clock::now() < target) {
			YieldProcessor();
		}

		// 処理落ちして大きく遅れている場合は、遅れを取り戻そうとせず現在時刻から数え直す
		now = Clock::now();
		reference_ = (now - target > kFrameTime) ? now : target;
	}

	// 今フレームが使うスロットのGPU処理が終わるまで待ち、そのスロット用のリソースへ切り替える
	void DirectXCommon::WaitForFrameSlot() {
		frameSync_->WaitFor(frameFenceValues_[frameIndex_]);
		frameContext_->frameIndex = frameIndex_;
		constantBufferAllocator_->BeginFrame(frameIndex_);
	}

	// GPUの処理がすべて完了するまで待つ
	void DirectXCommon::WaitIdle() {
		frameSync_->SignalAndWait(commandContext_->GetCommandQueue());
	}

	void DirectXCommon::BeginFrame() {
		swapChain_->UpdateBackBufferIndex();

		commandContext_->Reset(frameIndex_); // このスロットのGPU処理はWaitForFrameSlot()で完了済み

		ID3D12GraphicsCommandList* commandList = commandContext_->GetCommandList();

		// 今フレームで使うコマンドリストをFrameContextへ反映する
		frameContext_->commandList = commandList;

		// バリア: Present → RenderTarget
		swapChain_->TransitionToRenderTarget(commandList);

		// RTV / DSV
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain_->GetCurrentRTVHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

		// クリア
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
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

		// フェンスをシグナルして、このスロットのGPU完了を示す値を記録する（待つのは、次にこのスロットを使う時）
		frameFenceValues_[frameIndex_] = frameSync_->Signal(commandContext_->GetCommandQueue());

		// Present
		swapChain_->Present();

		// 次のフレームのスロットへ進む
		frameIndex_ = (frameIndex_ + 1) % kFrameCount;

		//FPS固定
		UpdateFixFPS();
	}

	// 記録済みのコマンドを即座にGPUへ投入し、完了するまで待つ（初期化処理など、次の描画フレームを待たずに結果が必要な場合に使う）
	void DirectXCommon::FlushCommands() {
		commandContext_->CloseAndExecute();
		frameSync_->SignalAndWait(commandContext_->GetCommandQueue());
		// リセットは次の BeginFrame() に委ねる（Close 済みなので安全）
	}

	// フレーム外でコマンドを記録できるよう、Close済みのコマンドリストをリセットして開く
	void DirectXCommon::BeginUpload() {
		commandContext_->Reset(frameIndex_); // このスロットのGPU処理はWaitForFrameSlot()で完了済み
		frameContext_->commandList = commandContext_->GetCommandList();
	}

	void DirectXCommon::Finalize() {
		// Present後にDXGIが投入したGPU作業（バックバッファのcomposite等）も含めて完了を待つ
		frameSync_->SignalAndWait(commandContext_->GetCommandQueue());
		frameSync_->Finalize();
		if (limiterTimer_) {
			CloseHandle(limiterTimer_);
			limiterTimer_ = nullptr;
		}

		// コマンドリスト解放後にダングリングポインタが残らないようにする
		frameContext_->commandList = nullptr;
		frameContext_->constantBuffers = nullptr;
		constantBufferAllocator_.reset();

		// シングルトンの ComPtr は main() 返却後まで破棄されないため、明示的に解放してからリークチェックを行う
		depthStencilResource_.Reset();
		dsvDescriptorHeap_.Reset();
		srvDescriptorHeap_.Reset();
		swapChain_->Finalize();
		commandContext_->Finalize();
		device_->Finalize();
	}

} // namespace Engine

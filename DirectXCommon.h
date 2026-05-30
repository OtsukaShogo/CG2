#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h> 
#include <cstdint>

class DirectXCommon {
public:

    static DirectXCommon* GetInstance();

    //コピー禁止
    DirectXCommon(const DirectXCommon&) = delete;
    DirectXCommon& operator=(const DirectXCommon&) = delete;

    void Initialize(HWND hwnd, uint32_t width, uint32_t height);
    void BeginFrame();
    void EndFrame();
    void Finalize();

    // 初期化時など、記録済みコマンドをGPUへ投入して完了を待つ
    void FlushCommands();

    ID3D12Device* GetDevice() const { return device_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap_.Get(); }

    uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV_; }
    uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV_; }
    uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV_; }

private:
    DirectXCommon() = default;
    ~DirectXCommon() = default;

    void InitializeDevice();
    void InitializeCommand();
    void InitializeSwapChain(HWND hwnd, uint32_t width, uint32_t height);
    void InitializeRenderTargets();
    void InitializeDepthStencil(uint32_t width, uint32_t height);
    void InitializeFence();

private:
    static const uint32_t kBackBufferCount = 2;

    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[kBackBufferCount];
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kBackBufferCount]{};

    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;

    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    uint32_t backBufferIndex_ = 0;

    uint32_t descriptorSizeSRV_ = 0;
    uint32_t descriptorSizeRTV_ = 0;
    uint32_t descriptorSizeDSV_ = 0;

    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissorRect_{};
};

#include "SwapChain.h"
#include <cassert>

#include "D3D12Util.h"

namespace Engine {

void SwapChain::Initialize(IDXGIFactory7* factory, ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND hwnd, uint32_t width, uint32_t height) {
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = kBackBufferCount;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    hr = factory->CreateSwapChainForHwnd(
        commandQueue, hwnd, &swapChainDesc,
        nullptr, nullptr, &swapChain1);
    assert(SUCCEEDED(hr));

    hr = swapChain1.As(&swapChain_);
    assert(SUCCEEDED(hr));

    // バックバッファ取得
    for (uint32_t i = 0; i < kBackBufferCount; ++i) {
        hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
        assert(SUCCEEDED(hr));
    }

    // RTVヒープ
    rtvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kBackBufferCount, false);
    uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // RTV作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

    for (uint32_t i = 0; i < kBackBufferCount; ++i) {
        rtvHandles_[i] = rtvStartHandle;
        rtvHandles_[i].ptr += static_cast<SIZE_T>(descriptorSizeRTV) * i;
        device->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc, rtvHandles_[i]);
    }
}

void SwapChain::UpdateBackBufferIndex() {
    backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}

void SwapChain::TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList) {
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
}

void SwapChain::TransitionToPresent(ID3D12GraphicsCommandList* commandList) {
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
}

void SwapChain::Present() {
    swapChain_->Present(1, 0);
}

void SwapChain::Finalize() {
    rtvDescriptorHeap_.Reset();
    for (auto& res : swapChainResources_) { res.Reset(); }
    swapChain_.Reset();
}

} // namespace Engine

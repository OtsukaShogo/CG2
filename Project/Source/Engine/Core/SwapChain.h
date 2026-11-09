#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>

namespace Engine {

/// <summary>
/// スワップチェーン・バックバッファ・RTVディスクリプタヒープの管理と、
/// バックバッファに対するPresent⇔RenderTarget間のバリア遷移を担うクラス
/// </summary>
class SwapChain {
public:
    static const uint32_t kBackBufferCount = 2;

    SwapChain() = default;
    ~SwapChain() = default;

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    /// <summary>
    /// スワップチェーンとRTVディスクリプタヒープ・RTVを生成する
    /// </summary>
    /// <param name="factory">DXGIファクトリ</param>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="commandQueue">スワップチェーンに紐づけるコマンドキュー</param>
    /// <param name="hwnd">対象のウィンドウハンドル</param>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    void Initialize(IDXGIFactory7* factory, ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND hwnd, uint32_t width, uint32_t height);

    /// <summary>
    /// 終了処理を行う（バックバッファ・RTVヒープ・スワップチェーンの解放）
    /// </summary>
    void Finalize();

    /// <summary>
    /// 現在のバックバッファインデックスを更新する
    /// </summary>
    void UpdateBackBufferIndex();

    /// <summary>
    /// 現在のバックバッファをPresent状態からRenderTarget状態へ遷移させる
    /// </summary>
    /// <param name="commandList">記録先のコマンドリスト</param>
    void TransitionToRenderTarget(ID3D12GraphicsCommandList* commandList);

    /// <summary>
    /// 現在のバックバッファをRenderTarget状態からPresent状態へ遷移させる
    /// </summary>
    /// <param name="commandList">記録先のコマンドリスト</param>
    void TransitionToPresent(ID3D12GraphicsCommandList* commandList);

    /// <summary>
    /// 画面に表示する（Present）
    /// </summary>
    void Present();

    /// <summary>
    /// 現在のバックバッファのRTVハンドルを取得する
    /// </summary>
    /// <returns>現在のバックバッファのRTVハンドル</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTVHandle() const { return rtvHandles_[backBufferIndex_]; }

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[kBackBufferCount];
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kBackBufferCount]{};

    uint32_t backBufferIndex_ = 0;
};

} // namespace Engine

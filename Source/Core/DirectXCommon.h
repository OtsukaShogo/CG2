#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>

#include "D3D12Device.h"
#include "CommandContext.h"
#include "SwapChain.h"
#include "FrameSync.h"

namespace Engine {

/// <summary>
/// DirectX12のデバイス・コマンド・スワップチェーンなど描画基盤全般を統括するファサードクラス。
/// 実体はD3D12Device / CommandContext / SwapChain / FrameSyncの各クラスに委譲する。
/// </summary>
class DirectXCommon {
public:
    DirectXCommon() = default;
    ~DirectXCommon() = default;

    //コピー禁止
    DirectXCommon(const DirectXCommon&) = delete;
    DirectXCommon& operator=(const DirectXCommon&) = delete;

    /// <summary>
    /// デバイス・コマンド・スワップチェーンなど描画に必要な各種リソースを初期化する
    /// </summary>
    /// <param name="hwnd">対象のウィンドウハンドル</param>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    void Initialize(HWND hwnd, uint32_t width, uint32_t height);

    /// <summary>
    /// 1フレームの描画開始処理（バリア設定・レンダーターゲットのクリアなど）を行う
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// 1フレームの描画終了処理（コマンド実行・画面表示・フェンス待機）を行う
    /// </summary>
    void EndFrame();

    /// <summary>
    /// 終了処理を行う
    /// </summary>
    void Finalize();

    /// <summary>
    /// 記録済みコマンドをGPUへ投入し、完了を待つ（初期化時など）
    /// </summary>
    void FlushCommands();

    /// <summary>
    /// D3D12デバイスを取得する
    /// </summary>
    /// <returns>D3D12デバイス</returns>
    ID3D12Device* GetDevice() const { return device_->Get(); }

    /// <summary>
    /// コマンドリストを取得する
    /// </summary>
    /// <returns>コマンドリスト</returns>
    ID3D12GraphicsCommandList* GetCommandList() const { return commandContext_->GetCommandList(); }

    /// <summary>
    /// SRV用ディスクリプタヒープを取得する
    /// </summary>
    /// <returns>SRV用ディスクリプタヒープ</returns>
    ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap_.Get(); }

    /// <summary>
    /// SRVディスクリプタ1個分のサイズを取得する
    /// </summary>
    /// <returns>SRVディスクリプタ1個分のサイズ</returns>
    uint32_t GetDescriptorSizeSRV() const { return descriptorSizeSRV_; }

    /// <summary>
    /// RTVディスクリプタ1個分のサイズを取得する
    /// </summary>
    /// <returns>RTVディスクリプタ1個分のサイズ</returns>
    uint32_t GetDescriptorSizeRTV() const { return descriptorSizeRTV_; }

    /// <summary>
    /// DSVディスクリプタ1個分のサイズを取得する
    /// </summary>
    /// <returns>DSVディスクリプタ1個分のサイズ</returns>
    uint32_t GetDescriptorSizeDSV() const { return descriptorSizeDSV_; }

private:
    /// <summary>
    /// 汎用SRVヒープと深度ステンシルリソース・DSVヒープを生成する
    /// </summary>
    /// <param name="width">深度ステンシルテクスチャの幅</param>
    /// <param name="height">深度ステンシルテクスチャの高さ</param>
    void InitializeHeapsAndDepthStencil(uint32_t width, uint32_t height);

private:
    std::unique_ptr<D3D12Device> device_;
    std::unique_ptr<CommandContext> commandContext_;
    std::unique_ptr<SwapChain> swapChain_;
    std::unique_ptr<FrameSync> frameSync_;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;

    uint32_t descriptorSizeSRV_ = 0;
    uint32_t descriptorSizeRTV_ = 0;
    uint32_t descriptorSizeDSV_ = 0;

    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissorRect_{};
};

} // namespace Engine

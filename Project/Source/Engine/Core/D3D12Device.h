#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Engine {

/// <summary>
/// DXGIファクトリ・アダプタ選択・D3D12デバイスの生成を担うクラス
/// </summary>
class D3D12Device {
public:
    D3D12Device() = default;
    ~D3D12Device() = default;

    D3D12Device(const D3D12Device&) = delete;
    D3D12Device& operator=(const D3D12Device&) = delete;

    /// <summary>
    /// DXGIファクトリ・アダプタ・D3D12デバイスを生成する（デバッグビルドではデバッグレイヤーも有効化する）
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理を行う（デバイス・アダプタ・ファクトリの解放）
    /// </summary>
    void Finalize();

    /// <summary>
    /// DXGIファクトリを取得する
    /// </summary>
    /// <returns>DXGIファクトリ</returns>
    IDXGIFactory7* GetFactory() const { return dxgiFactory_.Get(); }

    /// <summary>
    /// D3D12デバイスを取得する
    /// </summary>
    /// <returns>D3D12デバイス</returns>
    ID3D12Device* Get() const { return device_.Get(); }

private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
};

} // namespace Engine

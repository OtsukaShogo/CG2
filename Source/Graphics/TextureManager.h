#pragma once
#include <string>
#include <d3d12.h>
#include <wrl/client.h>

namespace Engine {

/// <summary>
/// 読み込み済みテクスチャのリソースとSRVハンドルをまとめた構造体
/// </summary>
struct TextureHandle {
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;      // 実テクスチャ
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediate; // アップロード用
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};             // SRV の CPU ハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};             // SRV の GPU ハンドル
};

/// <summary>
/// テクスチャファイルの読み込みとSRV生成を行うクラス
/// </summary>
class TextureManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    /// <returns>TextureManagerのインスタンス</returns>
    [[nodiscard]] static TextureManager* GetInstance();

    /// <summary>
    /// テクスチャファイルを読み込み、GPUへ転送してSRVを作成しハンドルを返す
    /// </summary>
    /// <param name="filePath">読み込むテクスチャファイルのパス</param>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="commandList">アップロードに使うコマンドリスト</param>
    /// <param name="srvHeap">SRV用ディスクリプタヒープ</param>
    /// <param name="descriptorSizeSRV">SRVディスクリプタ1個分のサイズ</param>
    /// <returns>読み込んだテクスチャのハンドル</returns>
    [[nodiscard]] TextureHandle LoadTexture(
        const std::string& filePath,
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12DescriptorHeap* srvHeap,
        uint32_t descriptorSizeSRV);

    /// <summary>
    /// SRV用ディスクリプタヒープ内の空きインデックスを1つ割り当てる
    /// （テクスチャ以外のSRV・StructuredBufferなどでも、ヒープ内でインデックスが重複しないよう共通で利用する）
    /// </summary>
    /// <returns>割り当てたインデックス</returns>
    [[nodiscard]] uint32_t AllocateSrvIndex() { return srvIndex_++; }

private:
    TextureManager() = default;
    ~TextureManager() = default;

private:
    uint32_t srvIndex_ = 1; // 0 は ImGui が使うので 1 から
};

} // namespace Engine

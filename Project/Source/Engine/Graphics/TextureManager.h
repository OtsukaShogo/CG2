#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <d3d12.h>
#include <wrl/client.h>

#include "DescriptorAllocator.h"

namespace Engine {

/// <summary>
/// 確保したSRVスロットの所有権。最後のコピーが破棄されると、スロットが自動でヒープへ返却される
/// （返却のタイミングでGPUが使用中でないことは、呼び出し側が保証する）
/// </summary>
using SrvSlot = std::shared_ptr<const uint32_t>;

/// <summary>
/// 読み込み済みテクスチャのリソースとSRVハンドルをまとめた構造体。
/// コピーするとSRVスロットを共有し、すべてのコピーが破棄されたときにスロットが解放される
/// </summary>
struct TextureHandle {
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;      // 実テクスチャ
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediate; // アップロード用
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};             // SRV の CPU ハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};             // SRV の GPU ハンドル
    SrvSlot srvSlot;                                     // SRV スロットの所有権（解放用）
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
    /// SRVスロット管理を初期化する（SRVヒープ作成後、他のSRVを確保する前に呼ぶ）
    /// </summary>
    /// <param name="heapCapacity">SRVヒープ全体のディスクリプタ数</param>
    void Initialize(uint32_t heapCapacity);

    /// <summary>
    /// SRV用ディスクリプタヒープ内の空きスロットを1つ確保する
    /// （テクスチャ以外のSRV・StructuredBufferなどでも、ヒープ内でインデックスが重複しないよう共通で利用する）。
    /// 返したSlotの最後のコピーが破棄されるとスロットは自動で解放される
    /// </summary>
    /// <returns>確保したスロット。*slot がヒープ内のインデックス</returns>
    [[nodiscard]] SrvSlot AllocateSrv();

    /// <summary>
    /// 現在確保中のSRVスロット数を取得する（リーク確認用）
    /// </summary>
    [[nodiscard]] uint32_t GetUsedSrvCount() const { return allocator_.GetUsedCount(); }

private:
    TextureManager() = default;
    ~TextureManager() = default;

private:
    DescriptorAllocator allocator_;
};

} // namespace Engine

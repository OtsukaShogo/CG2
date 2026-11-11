#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
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
/// コピーするとリソースとSRVスロットを共有し、すべてのコピーが破棄されたときに解放される
/// </summary>
struct TextureHandle {
    SrvSlot srvSlot;                                     // SRV スロットの所有権（解放用）
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;      // 実テクスチャ
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};             // SRV の CPU ハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};             // SRV の GPU ハンドル
};

/// <summary>
/// テクスチャファイルの読み込みとSRV生成を行うクラス。
/// 同じファイルは読み込み済みのものを共有し、デフォルトテクスチャ（白）も提供する
/// </summary>
class TextureManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    /// <returns>TextureManagerのインスタンス</returns>
    [[nodiscard]] static TextureManager* GetInstance();

    /// <summary>
    /// SRVスロット管理を初期化する（SRVヒープ作成後、他のSRVを確保する前に呼ぶ）
    /// </summary>
    /// <param name="heapCapacity">SRVヒープ全体のディスクリプタ数</param>
    void Initialize(uint32_t heapCapacity);

    /// <summary>
    /// 保持しているテクスチャ（デフォルトテクスチャ・キャッシュ）を解放する。デバイス解放前に呼ぶ
    /// </summary>
    void Finalize();

    /// <summary>
    /// テクスチャファイルを読み込み、GPUへ転送してSRVを作成しハンドルを返す。
    /// 同じファイルが読み込み済み（かつどこかで使用中）なら、それを共有して返す。
    /// 読み込みに失敗した場合はログを出し、白テクスチャを返す。
    /// 読み込み時にミップマップも生成する
    /// </summary>
    /// <param name="filePath">読み込むテクスチャファイルのパス（UTF-8）</param>
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
    /// デフォルトテクスチャ（1x1の白）を作成する。Framework初期化時に1度だけ呼ぶ
    /// </summary>
    void CreateDefaultTextures(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12DescriptorHeap* srvHeap,
        uint32_t descriptorSizeSRV);

    /// <summary>
    /// 白い1x1のテクスチャを取得する（テクスチャなしで色だけで描画したいときに使う）
    /// </summary>
    [[nodiscard]] const TextureHandle& GetWhiteTexture() const { return whiteTexture_; }

    /// <summary>
    /// アップロード用の中間バッファを解放する。
    /// GPUへの転送コマンドが完了した後（FlushCommands の後）に呼ぶこと
    /// </summary>
    void ReleaseUploadBuffers();

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

    // 読み込み済みテクスチャをキャッシュするための情報。
    // 所有はしない（所有しているのはTextureHandleのコピー）ので、誰も使わなくなれば自動的に失効する
    struct CacheEntry {
        std::weak_ptr<const uint32_t> slot; // 失効していたら使われていない
        ID3D12Resource* texture = nullptr;  // slotが有効な間は、どこかのTextureHandleが生かしている
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
    };

    // 画像データ（DirectX::ScratchImage）からテクスチャ・SRVを作成する（キャッシュには登録しない）
    TextureHandle CreateFromImage(
        const void* scratchImage,
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12DescriptorHeap* srvHeap,
        uint32_t descriptorSizeSRV);

private:
    DescriptorAllocator allocator_;
    std::unordered_map<std::wstring, CacheEntry> cache_;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> pendingUploadBuffers_;
    TextureHandle whiteTexture_;
};

} // namespace Engine

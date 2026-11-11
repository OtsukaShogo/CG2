#include "TextureManager.h"
#include "../Core/D3D12Util.h"
#include "../Core/ConvertString.h"
#include "../Core/DebugUtil.h"
#include "External/DirectXTex/DirectXTex.h"
#include <cassert>
#include <cstring>
#include <cwctype>
#include <filesystem>

namespace Engine {

TextureManager* TextureManager::GetInstance() {
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize(uint32_t heapCapacity) {
    allocator_.Initialize(heapCapacity, 1); // 0 は ImGui が使うので 1 から
    cache_.clear();
    pendingUploadBuffers_.clear();
    whiteTexture_ = {};
}

void TextureManager::Finalize() {
    cache_.clear();
    pendingUploadBuffers_.clear();
    whiteTexture_ = {};
}

SrvSlot TextureManager::AllocateSrv() {
    uint32_t* index = new uint32_t(allocator_.Allocate());
    // 最後の所有者が破棄されたらスロットをアロケータへ返す（アロケータはシングルトンの一部なので破棄時にも有効）
    return SrvSlot(index, [this](const uint32_t* p) {
        allocator_.Free(*p);
        delete p;
    });
}

void TextureManager::ReleaseUploadBuffers() {
    pendingUploadBuffers_.clear();
}

TextureHandle TextureManager::CreateFromImage(
    const void* scratchImage,
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12DescriptorHeap* srvHeap,
    uint32_t descriptorSizeSRV) {
    const DirectX::ScratchImage& mipImages = *static_cast<const DirectX::ScratchImage*>(scratchImage);
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    TextureHandle handle{};

    // テクスチャリソース作成
    handle.texture = CreateTextureResource(device, metadata);

    // アップロード（中間バッファは、GPUの転送完了後に ReleaseUploadBuffers で解放する）
    pendingUploadBuffers_.push_back(UploadTextureData(handle.texture.Get(), mipImages, device, commandList));

    // SRV の空きスロットを自動割り当て
    handle.srvSlot = AllocateSrv();
    uint32_t index = *handle.srvSlot;

    handle.cpuHandle = GetCPUDescriptorHandle(srvHeap, descriptorSizeSRV, index);
    handle.gpuHandle = GetGPUDescriptorHandle(srvHeap, descriptorSizeSRV, index);

    // SRV 作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    device->CreateShaderResourceView(handle.texture.Get(), &srvDesc, handle.cpuHandle);

    return handle;
}

void TextureManager::CreateDefaultTextures(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12DescriptorHeap* srvHeap,
    uint32_t descriptorSizeSRV) {
    DirectX::ScratchImage white{};
    HRESULT hr = white.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, 1, 1, 1);
    assert(SUCCEEDED(hr));
    std::memset(white.GetPixels(), 0xFF, white.GetPixelsSize());

    whiteTexture_ = CreateFromImage(&white, device, commandList, srvHeap, descriptorSizeSRV);
}

TextureHandle TextureManager::LoadTexture(
    const std::string& filePath,
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12DescriptorHeap* srvHeap,
    uint32_t descriptorSizeSRV) {
    // UTF-8 のパスをワイド文字へ変換（日本語パス対応）。キャッシュのキーは表記ゆれ・大文字小文字をそろえる
    std::wstring pathW = ConvertString(filePath);
    std::wstring key = std::filesystem::path(pathW).lexically_normal().generic_wstring();
    for (wchar_t& c : key) {
        c = static_cast<wchar_t>(std::towlower(c));
    }

    // 読み込み済みで、まだどこかで使われているテクスチャなら共有する
    if (auto it = cache_.find(key); it != cache_.end()) {
        if (SrvSlot slot = it->second.slot.lock()) {
            TextureHandle handle{};
            handle.srvSlot = std::move(slot);
            handle.texture = it->second.texture;
            handle.cpuHandle = it->second.cpuHandle;
            handle.gpuHandle = it->second.gpuHandle;
            return handle;
        }
        cache_.erase(it); // 失効したエントリは捨てる
    }

    // 画像読み込み
    DirectX::ScratchImage image{};
    HRESULT hr = DirectX::LoadFromWICFile(pathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    if (FAILED(hr)) {
        DebugUtil::Log("[TextureManager] テクスチャの読み込みに失敗しました: " + filePath + "\n");
        return whiteTexture_;
    }

    // ミップマップ生成（失敗した場合はミップなしのまま使う）
    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(
        image.GetImages(), image.GetImageCount(), image.GetMetadata(),
        DirectX::TEX_FILTER_SRGB, 0, mipImages);
    const DirectX::ScratchImage& source = SUCCEEDED(hr) ? mipImages : image;

    TextureHandle handle = CreateFromImage(&source, device, commandList, srvHeap, descriptorSizeSRV);

    CacheEntry entry;
    entry.slot = handle.srvSlot;
    entry.texture = handle.texture.Get();
    entry.cpuHandle = handle.cpuHandle;
    entry.gpuHandle = handle.gpuHandle;
    cache_[key] = entry;

    return handle;
}

} // namespace Engine

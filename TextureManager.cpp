#include "TextureManager.h"
#include "DirectXCommon.h"
#include "D3D12Util.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <cassert>

TextureManager* TextureManager::GetInstance() {
    static TextureManager instance;
    return &instance;
}

TextureHandle TextureManager::LoadTexture(const std::string& filePath) {
    TextureHandle handle{};

    // 1. 画像読み込み
    DirectX::ScratchImage mipImages{};
    {
        std::wstring pathW(filePath.begin(), filePath.end());
        HRESULT hr = DirectX::LoadFromWICFile(
            pathW.c_str(),
            DirectX::WIC_FLAGS_FORCE_SRGB,
            nullptr,
            mipImages
        );
        assert(SUCCEEDED(hr));
    }

    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    // 2. DX12 オブジェクト取得
    DirectXCommon* dx = DirectXCommon::GetInstance();
    ID3D12Device* device = dx->GetDevice();
    ID3D12GraphicsCommandList* commandList = dx->GetCommandList();
    ID3D12DescriptorHeap* srvHeap = dx->GetSrvDescriptorHeap();

    // 3. テクスチャリソース作成
    handle.texture = CreateTextureResource(device, metadata);

    // 4. アップロード
    handle.intermediate = UploadTextureData(handle.texture.Get(), mipImages, device, commandList);

    // 5. SRV の空きスロットを自動割り当て
    uint32_t index = srvIndex_++;
    uint32_t descriptorSize = dx->GetDescriptorSizeSRV();

    handle.cpuHandle = GetCPUDescriptorHandle(srvHeap, descriptorSize, index);
    handle.gpuHandle = GetGPUDescriptorHandle(srvHeap, descriptorSize, index);

    // 6. SRV 作成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    device->CreateShaderResourceView(handle.texture.Get(), &srvDesc, handle.cpuHandle);

    return handle;
}

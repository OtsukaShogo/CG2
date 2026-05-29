#pragma once
#include <string>
#include <d3d12.h>
#include <wrl/client.h>

struct TextureHandle {
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;      // 実テクスチャ
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediate; // アップロード用
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};             // SRV の CPU ハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};             // SRV の GPU ハンドル
};

class TextureManager {
public:
    static TextureManager* GetInstance();

    // テクスチャ読み込み → GPU転送 → SRV作成 → ハンドル返却
    TextureHandle LoadTexture(const std::string& filePath);

private:
    TextureManager() = default;
    ~TextureManager() = default;

private:
    uint32_t srvIndex_ = 1; // 0 は ImGui が使うので 1 から
};

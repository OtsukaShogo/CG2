#pragma once
#include<cstdint>
#include <d3d12.h>
#include <wrl/client.h> 
#include<string>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "TextureManager.h"

class Material {
public:
    struct ConstantData {
        Vector4   color          = { 1.0f, 1.0f, 1.0f, 1.0f };
        int32_t   enableLighting = 1;
        float     pad[3]         = {};  // float4x4 を 16byte 境界に揃えるためのパディング
        Matrix4x4 uvTransform   = {};
    };

public:
    Material();
    ~Material();

    void Create();
    void Bind(ID3D12GraphicsCommandList* commandList);

    void LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
    const std::string& GetTextureFilePath() const { return textureFilePath_; }

    void SetColor(const Vector4& color);
    Vector4&   GetColor()       { return materialData_->color; }
    Matrix4x4& GetUVTransform() { return materialData_->uvTransform; }
    void SetEnableLighting(bool enable) { materialData_->enableLighting = enable ? 1 : 0; }
    void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureSrvHandleGPU_ = handle; }

    // テクスチャを保持する（GPUへのアップロードが完了するまでtexture_/intermediateを生かしておく必要があるため）
    void SetTexture(const TextureHandle& texture) {
        texture_ = texture;
        textureSrvHandleGPU_ = texture.gpuHandle;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    ConstantData* materialData_ = nullptr;

    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};

    std::string   textureFilePath_;
    TextureHandle texture_;
};
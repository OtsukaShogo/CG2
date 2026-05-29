#pragma once
#include <d3d12.h>
#include <wrl/client.h> 
#include "Vector4.h"

class Material {
public:
    struct ConstantData {
        Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

public:
    Material();
    ~Material();

    void Create();
    void Bind(ID3D12GraphicsCommandList* commandList);

    void SetColor(const Vector4& color);
    Vector4& GetColor() { return materialData_->color; }
    void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureSrvHandleGPU_ = handle; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    ConstantData* materialData_ = nullptr;

    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
};
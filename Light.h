#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "Vector4.h"
#include "Vector3.h"

class Light {
public:
    struct DirectionalLight {
        Vector4 color     = { 1.0f, 1.0f, 1.0f, 1.0f };
        Vector3 direction = { 0.0f, -1.0f, 0.0f };
        float   intensity = 1.0f;
    };

    Light();
    ~Light();

    void Initialize(ID3D12Device* device);

    DirectionalLight* GetDirectionalLight() const { return directionalLightData_; }
    D3D12_GPU_VIRTUAL_ADDRESS GetDirectionalLightAddress() const { return directionalLightResource_->GetGPUVirtualAddress(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
    DirectionalLight* directionalLightData_ = nullptr;
};

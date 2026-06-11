#include "Light.h"
#include "D3D12Util.h"

Light::Light(){}

Light::~Light() {
    if (directionalLightResource_) {
        directionalLightResource_->Unmap(0, nullptr);
    }
}

void Light::Initialize(ID3D12Device* device) {
    directionalLightResource_ = CreateBufferResource(device, sizeof(DirectionalLight));
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    *directionalLightData_ = DirectionalLight{};
}

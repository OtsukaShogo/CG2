#include "Light.h"
#include "D3D12Util.h"

namespace Engine {

Light::Light(){}

Light::~Light() {
    if (directionalLightResource_) {
        directionalLightResource_->Unmap(0, nullptr);
    }
}

// 定数バッファを作成してCPUから書き込める状態にし、デフォルト値を書き込んでおく
void Light::Initialize(ID3D12Device* device) {
    directionalLightResource_ = CreateBufferResource(device, sizeof(DirectionalLight));
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    *directionalLightData_ = DirectionalLight{};
}

} // namespace Engine

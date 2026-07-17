#include "AssetFactory.h"
#include "DirectXCommon.h"

namespace Engine {

void AssetFactory::Initialize(DirectXCommon* dx) {
    device_ = dx->GetDevice();
    commandList_ = dx->GetCommandList();
    srvHeap_ = dx->GetSrvDescriptorHeap();
    descriptorSizeSRV_ = dx->GetDescriptorSizeSRV();
}

std::shared_ptr<ModelData> AssetFactory::CreateModelData(const std::string& directoryPath, const std::string& filename) {
    return ModelData::CreateFromObj(directoryPath, filename, device_, commandList_, srvHeap_, descriptorSizeSRV_);
}

std::shared_ptr<ModelData> AssetFactory::CreateSphereModelData() {
    return ModelData::CreateSphere(device_);
}

std::unique_ptr<Model> AssetFactory::CreateModelInstance(std::shared_ptr<const ModelData> modelData) {
    return Model::CreateInstance(std::move(modelData), device_);
}

std::unique_ptr<Sprite> AssetFactory::CreateSprite(float width, float height) {
    return Sprite::Create(width, height, device_);
}

TextureHandle AssetFactory::LoadTexture(const std::string& filePath) {
    return TextureManager::GetInstance()->LoadTexture(filePath, device_, commandList_, srvHeap_, descriptorSizeSRV_);
}

} // namespace Engine

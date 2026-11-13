#include "AssetFactory.h"
#include "../Core/DirectXCommon.h"

namespace Engine {

void AssetFactory::Initialize(DirectXCommon* dx) {
    device_ = dx->GetDevice();
    srvHeap_ = dx->GetSrvDescriptorHeap();
    descriptorSizeSRV_ = dx->GetDescriptorSizeSRV();
    frameContext_ = dx->GetFrameContext();
}

std::shared_ptr<ModelData> AssetFactory::CreateModelData(const std::string& directoryPath, const std::string& filename) {
    return ModelData::CreateFromObj(directoryPath, filename, device_, frameContext_->commandList, srvHeap_, descriptorSizeSRV_);
}

std::shared_ptr<ModelData> AssetFactory::CreatePrimitiveModelData(PrimitiveType type) {
    return ModelData::CreatePrimitive(type, device_);
}

std::shared_ptr<ModelData> AssetFactory::CreateSphereModelData() {
    return ModelData::CreateSphere(device_);
}

std::unique_ptr<Model> AssetFactory::CreateModelInstance(std::shared_ptr<const ModelData> modelData) {
    return Model::CreateInstance(std::move(modelData), frameContext_);
}

std::unique_ptr<Sprite> AssetFactory::CreateSprite(float width, float height) {
    return Sprite::Create(width, height, device_, frameContext_);
}

TextureHandle AssetFactory::LoadTexture(const std::string& filePath) {
    return TextureManager::GetInstance()->LoadTexture(filePath, device_, frameContext_->commandList, srvHeap_, descriptorSizeSRV_);
}

} // namespace Engine

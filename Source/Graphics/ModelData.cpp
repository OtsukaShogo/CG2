#include "ModelData.h"

namespace Engine {

std::shared_ptr<ModelData> ModelData::CreateFromObj(
    const std::string& directoryPath,
    const std::string& filename,
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12DescriptorHeap* srvHeap,
    uint32_t descriptorSizeSRV) {
    auto modelData = std::make_shared<ModelData>();

    // Mesh生成（OBJファイル読み込み）・GPU転送
    modelData->mesh_.LoadObjFile(directoryPath, filename);
    modelData->mesh_.Upload(device);

    // OBJのmtlで指定されたテクスチャを読み込む
    // （GPUへのアップロードが完了するまでtexture/intermediateを生かしておく必要があるため、ModelData側で保持する）
    modelData->texture_ = TextureManager::GetInstance()->LoadTexture(
        modelData->mesh_.GetTextureFilePath(), device, commandList, srvHeap, descriptorSizeSRV);

    return modelData;
}

std::shared_ptr<ModelData> ModelData::CreateSphere(ID3D12Device* device) {
    auto modelData = std::make_shared<ModelData>();

    modelData->mesh_.CreateSphere();
    modelData->mesh_.Upload(device);

    return modelData;
}

} // namespace Engine

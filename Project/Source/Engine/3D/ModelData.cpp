#include "ModelData.h"

#include "../Core/DebugUtil.h"

namespace Engine {

std::shared_ptr<ModelData> ModelData::CreateFromObj(
    const std::string& directoryPath,
    const std::string& filename,
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    ID3D12DescriptorHeap* srvHeap,
    uint32_t descriptorSizeSRV) {
    auto modelData = std::make_shared<ModelData>();

    // Mesh生成（OBJファイル読み込み）。読み込めなかった場合は、見た目で気付けるよう球で代用する
    if (!modelData->mesh_.LoadObjFile(directoryPath, filename)) {
        DebugUtil::Log("[ModelData] モデルを読み込めなかったため、球で代用します: " + directoryPath + "/" + filename + "\n");
        modelData->mesh_.CreateSphere();
    }
    modelData->mesh_.Upload(device);

    // OBJのmtlで指定されたテクスチャを読み込む。指定がない場合は白テクスチャを使う
    // （GPUへのアップロードが完了するまでtextureを生かしておく必要があるため、ModelData側で保持する）
    const std::string& texturePath = modelData->mesh_.GetTextureFilePath();
    if (texturePath.empty()) {
        modelData->texture_ = TextureManager::GetInstance()->GetWhiteTexture();
    } else {
        modelData->texture_ = TextureManager::GetInstance()->LoadTexture(
            texturePath, device, commandList, srvHeap, descriptorSizeSRV);
    }

    return modelData;
}

std::shared_ptr<ModelData> ModelData::CreateSphere(ID3D12Device* device) {
    auto modelData = std::make_shared<ModelData>();

    modelData->mesh_.CreateSphere();
    modelData->mesh_.Upload(device);
    modelData->texture_ = TextureManager::GetInstance()->GetWhiteTexture(); // 色だけで描画できるように白を使う

    return modelData;
}

} // namespace Engine

#include "Material.h"
#include "../Core/D3D12Util.h"
#include "../Math/Matrix4x4.h"
#include "../Core/PipelineStateManager.h"

#include "../Core/ConvertString.h"
#include "../Core/DebugUtil.h"

#include<cassert>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace Engine {

Material::Material() {
    materialData_.uvTransform = MakeIdentity4x4();
}

Material::~Material() = default;

void Material::SetColor(const Vector4& color) {
    materialData_.color = color;
}

// マテリアルの定数バッファ（描画ごとに切り出して書き込む）とテクスチャを、それぞれ対応するルートパラメータにセットする
void Material::Bind(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers) {
    commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kMaterial), constantBuffers.Upload(materialData_));
    commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kTexture), textureSrvHandleGPU_);
}

bool Material::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    //1.中で必要となる変数の宣言
    std::string line; // ファイルから読んだ1行を格納するもの
    std::ifstream file(std::filesystem::path(ConvertString(directoryPath + "/" + filename))); // ファイルを開く
    if (!file.is_open()) {
        // 開けなくても止めない（テクスチャなしとして扱われる）
        DebugUtil::Log("[Material] mtlファイルを開けません: " + directoryPath + "/" + filename + "\n");
        return false;
    }

    //2.実際にファイルを読み、textureFilePathに代入する
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierに応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            // 連結してファイルパスにする
            textureFilePath_ = directoryPath + "/" + textureFilename;
        }
    }
    return true;
}

} // namespace Engine
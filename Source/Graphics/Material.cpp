#include "Material.h"
#include "D3D12Util.h"
#include "Matrix4x4.h"
#include "PipelineStateManager.h"

#include<cassert>
#include <fstream>
#include <sstream>

namespace Engine {

Material::Material(){}

Material::~Material() {
    if (materialResource_) {
        materialResource_->Unmap(0, nullptr);
    }
}

// 定数バッファを作成してCPUから書き込める状態にし、デフォルト値を書き込んでおく
void Material::Create(ID3D12Device* device) {
    materialResource_ = CreateBufferResource(device, sizeof(ConstantData));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    *materialData_ = ConstantData{};
    materialData_->uvTransform = MakeIdentity4x4();
}

void Material::SetColor(const Vector4& color) {
    if (materialData_) materialData_->color = color;
}

// マテリアルの定数バッファとテクスチャを、それぞれ対応するルートパラメータにセットする
void Material::Bind(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kMaterial), materialResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kTexture), textureSrvHandleGPU_);
}

void Material::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    //1.中で必要となる変数の宣言
    std::string line; // ファイルから読んだ1行を格納するもの
    std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
    assert(file.is_open()); // とりあえず開けなかったら止める

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
}

} // namespace Engine
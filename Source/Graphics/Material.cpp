#include "Material.h"
#include "DirectXCommon.h"
#include "D3D12Util.h"
#include "Matrix4x4.h"

#include<cassert>
#include <fstream>
#include <sstream>

Material::Material(){}

Material::~Material() {
    if (materialResource_) {
        materialResource_->Unmap(0, nullptr);
    }
}

void Material::Create() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();

    materialResource_ = CreateBufferResource(device, sizeof(ConstantData));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    *materialData_ = ConstantData{};
    materialData_->uvTransform = MakeIdentity4x4();
}

void Material::SetColor(const Vector4& color) {
    if (materialData_) materialData_->color = color;
}

void Material::Bind(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
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
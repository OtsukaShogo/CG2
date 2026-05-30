#include "Material.h"
#include "DirectXCommon.h"
#include "D3D12_Util.h"

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
}

void Material::SetColor(const Vector4& color) {
    if (materialData_) materialData_->color = color;
}

void Material::Bind(ID3D12GraphicsCommandList* commandList) {
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
}
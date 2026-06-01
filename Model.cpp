#include "Model.h"
#include "DirectXCommon.h"
#include "D3D12_Util.h"
#include "TransformMatrix.h"

Model::Model() {}

Model::~Model() {
    if (wvpResource_) {
        wvpResource_->Unmap(0, nullptr);
    }
}

Model* Model::CreateSphere() {
    auto* model = new Model();

    // Mesh生成・GPU転送
    model->mesh_.CreateSphere();
    model->mesh_.Upload();

    // Material生成
    model->material_.Create();

    // WVP生成
    model->CreateWvpBuffer();

    return model;
}

void Model::CreateWvpBuffer() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    wvpResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
    wvpData_->WVP = MakeIdentity4x4();
    wvpData_->World = MakeIdentity4x4();
}

void Model::Update() {
    worldTransform_.rotate.y += 0.01f;
}

void Model::Draw(const Matrix4x4& viewProjection) {
    auto* commandList = DirectXCommon::GetInstance()->GetCommandList();

    Matrix4x4 worldMatrix = MakeAffineMatrix(
        worldTransform_.scale,
        worldTransform_.rotate,
        worldTransform_.translate
    );
    wvpData_->WVP = Multiply(worldMatrix, viewProjection);
    wvpData_->World = worldMatrix;

    commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

    material_.Bind(commandList);
    mesh_.Draw(commandList);
}
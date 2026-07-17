#include "Model.h"
#include "ModelData.h"
#include "D3D12Util.h"
#include "TransformMatrix.h"
#include "PipelineStateManager.h"

namespace Engine {

Model::Model() {}

Model::~Model() {
    if (wvpResource_) {
        wvpResource_->Unmap(0, nullptr);
    }
}

std::unique_ptr<Model> Model::CreateInstance(std::shared_ptr<const ModelData> modelData, ID3D12Device* device) {
    auto model = std::make_unique<Model>();

    // Material生成（インスタンス固有。色やUV変換を個別に持たせるため、モデルごとに作成する）
    model->material_.Create(device);
    model->material_.SetTexture(modelData->GetTexture());

    model->modelData_ = std::move(modelData);

    // WVP生成
    model->CreateWvpBuffer(device);

    return model;
}

// WVP行列用の定数バッファを作成してCPUから書き込める状態にし、単位行列で初期化しておく
void Model::CreateWvpBuffer(ID3D12Device* device) {
    wvpResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
    wvpData_->WVP = MakeIdentity4x4();
    wvpData_->World = MakeIdentity4x4();
}

void Model::Update() {
    worldTransform_.rotate.y += 0.01f;
}

void Model::Draw(const Matrix4x4& viewProjection, ID3D12GraphicsCommandList* commandList) {
    // ワールド行列を作り直し、ビュープロジェクションと合成してWVP定数バッファへ書き込む
    Matrix4x4 worldMatrix = MakeAffineMatrix(
        worldTransform_.scale,
        worldTransform_.rotate,
        worldTransform_.translate
    );
    wvpData_->WVP = Multiply(worldMatrix, viewProjection);
    wvpData_->World = worldMatrix;

    commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kWVP), wvpResource_->GetGPUVirtualAddress());

    material_.Bind(commandList);
    modelData_->GetMesh().Draw(commandList);
}

} // namespace Engine
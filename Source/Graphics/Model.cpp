#include "Model.h"
#include "DirectXCommon.h"
#include "D3D12Util.h"
#include "TransformMatrix.h"
#include "TextureManager.h"
#include "PipelineStateManager.h"

namespace Engine {

Model::Model() {}

Model::~Model() {
    if (wvpResource_) {
        wvpResource_->Unmap(0, nullptr);
    }
}

std::unique_ptr<Model> Model::CreateSphere() {
    auto model = std::make_unique<Model>();

    // Mesh生成・GPU転送
    model->mesh_.CreateSphere();
    model->mesh_.Upload();

    // Material生成
    model->material_.Create();

    // WVP生成
    model->CreateWvpBuffer();

    return model;
}

std::unique_ptr<Model> Model::CreateFromObj(const std::string& directoryPath, const std::string& filename) {
    auto model = std::make_unique<Model>();

    // Mesh生成（OBJファイル読み込み）・GPU転送
    model->mesh_.LoadObjFile(directoryPath, filename);
    model->mesh_.Upload();

    // Material生成
    model->material_.Create();

    // OBJのmtlで指定されたテクスチャを読み込み、バインドする
    // （GPUへのアップロードが完了するまでtexture/intermediateを生かしておく必要があるため、Material側で保持する）
    model->material_.SetTexture(TextureManager::GetInstance()->LoadTexture(model->mesh_.GetTextureFilePath()));

    // WVP生成
    model->CreateWvpBuffer();

    return model;
}

// WVP行列用の定数バッファを作成してCPUから書き込める状態にし、単位行列で初期化しておく
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
    mesh_.Draw(commandList);
}

} // namespace Engine
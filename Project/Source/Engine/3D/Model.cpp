#include "Model.h"
#include "ModelData.h"
#include "../Math/TransformMatrix.h"
#include "../Core/PipelineStateManager.h"
#include "../Graphics/ConstantBufferAllocator.h"

namespace Engine {

Model::Model() {}

Model::~Model() {}

std::unique_ptr<Model> Model::CreateInstance(
    std::shared_ptr<const ModelData> modelData,
    std::shared_ptr<const FrameContext> frameContext) {
    auto model = std::make_unique<Model>();

    // マテリアル（インスタンス固有。色やUV変換を個別に持たせる。GPUリソースは持たないので生成コストは小さい）
    model->material_.SetTexture(modelData->GetTexture());

    model->modelData_ = std::move(modelData);
    model->frameContext_ = std::move(frameContext);

    return model;
}

void Model::Draw(const Matrix4x4& viewProjection) {
    ID3D12GraphicsCommandList* commandList = frameContext_->commandList;
    ConstantBufferAllocator& constantBuffers = *frameContext_->constantBuffers;

    // ワールド行列を作り直し、ビュープロジェクションと合成してWVP定数バッファへ書き込む
    // （定数バッファは描画ごとにアロケータから切り出すので、同じフレーム内で何度描いても上書きされない）
    Matrix4x4 worldMatrix = MakeAffineMatrix(
        worldTransform_.scale,
        worldTransform_.rotate,
        worldTransform_.translate
    );
    TransformationMatrix wvp;
    wvp.WVP = Multiply(worldMatrix, viewProjection);
    wvp.World = worldMatrix;

    commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kWVP), constantBuffers.Upload(wvp));

    material_.Bind(commandList, constantBuffers);
    modelData_->GetMesh().Draw(commandList);
}

} // namespace Engine

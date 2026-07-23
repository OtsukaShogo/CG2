#include "Sprite.h"
#include "D3D12Util.h"
#include "TransformMatrix.h"
#include "PipelineStateManager.h"

namespace Engine {

Sprite::Sprite(){}

Sprite::~Sprite() {
    if (wvpResource_) {
        wvpResource_->Unmap(0, nullptr);
    }
}

std::unique_ptr<Sprite> Sprite::Create(
    float width, float height,
    ID3D12Device* device,
    std::shared_ptr<const FrameContext> frameContext) {
    auto sprite = std::make_unique<Sprite>();
    sprite->mesh_.CreateRect(width, height);
    sprite->mesh_.Upload(device);
    sprite->material_.Create(device);
    sprite->material_.SetEnableLighting(false);
    sprite->frameContext_ = std::move(frameContext);
    sprite->CreateWvpBuffer(device);
    return sprite;
}

// WVP行列用の定数バッファを作成してCPUから書き込める状態にし、単位行列で初期化しておく
void Sprite::CreateWvpBuffer(ID3D12Device* device) {
    wvpResource_ = CreateBufferResource(device, sizeof(TransformationMatrix));
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
    wvpData_->WVP = MakeIdentity4x4();
    wvpData_->World = MakeIdentity4x4();
}

void Sprite::Draw(const Matrix4x4& viewProjection) {
    ID3D12GraphicsCommandList* commandList = frameContext_->commandList;

    // worldTransform_ から行列を生成してからViewProjectionと合成
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
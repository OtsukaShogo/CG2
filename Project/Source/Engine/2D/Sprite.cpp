#include "Sprite.h"
#include "../Math/TransformMatrix.h"
#include "../Core/PipelineStateManager.h"
#include "../Graphics/ConstantBufferAllocator.h"

namespace Engine {

namespace {

// 全スプライトで共有する1x1の四角形メッシュ。使っているスプライトがなくなれば解放される
std::shared_ptr<const Mesh> GetSharedQuad(ID3D12Device* device) {
    static std::weak_ptr<const Mesh> cache;
    if (std::shared_ptr<const Mesh> mesh = cache.lock()) {
        return mesh;
    }
    auto mesh = std::make_shared<Mesh>();
    mesh->CreateRect(1.0f, 1.0f);
    mesh->Upload(device);
    cache = mesh;
    return mesh;
}

} // namespace

Sprite::Sprite(){}

Sprite::~Sprite() {}

std::unique_ptr<Sprite> Sprite::Create(
    float width, float height,
    ID3D12Device* device,
    std::shared_ptr<const FrameContext> frameContext) {
    auto sprite = std::make_unique<Sprite>();
    sprite->mesh_ = GetSharedQuad(device);
    sprite->size_ = { width, height };
    sprite->material_.SetEnableLighting(false);
    sprite->frameContext_ = std::move(frameContext);
    return sprite;
}

void Sprite::Draw(const Matrix4x4& viewProjection) {
    ID3D12GraphicsCommandList* commandList = frameContext_->commandList;
    ConstantBufferAllocator& constantBuffers = *frameContext_->constantBuffers;

    // worldTransform_ と大きさから行列を生成してからViewProjectionと合成
    // （メッシュは1x1なので、大きさはX・Yの拡大として反映する）
    Matrix4x4 worldMatrix = MakeAffineMatrix(
        { worldTransform_.scale.x * size_.x, worldTransform_.scale.y * size_.y, worldTransform_.scale.z },
        worldTransform_.rotate,
        worldTransform_.translate
    );
    TransformationMatrix wvp;
    wvp.WVP = Multiply(worldMatrix, viewProjection);
    wvp.World = worldMatrix;

    commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kWVP), constantBuffers.Upload(wvp));

    material_.Bind(commandList, constantBuffers);
    mesh_->Draw(commandList);
}

} // namespace Engine

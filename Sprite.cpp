#include "Sprite.h"
#include "DirectXCommon.h"
#include "D3D12_Util.h"
#include "TransformMatrix.h"

Sprite::Sprite(){}

Sprite::~Sprite() {
    if (wvpResource_) {
        wvpResource_->Unmap(0, nullptr);
    }
}

Sprite* Sprite::Create(float width, float height) {
    Sprite* sprite = new Sprite();
    sprite->mesh_.CreateRect(width, height);
    sprite->mesh_.Upload();
    sprite->material_.Create();
    sprite->CreateWvpBuffer();
    return sprite;
}

void Sprite::CreateWvpBuffer() {
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    wvpResource_ = CreateBufferResource(device, sizeof(Matrix4x4));
    wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
    *wvpData_ = MakeIdentity4x4();
}

void Sprite::Draw(const Matrix4x4& viewProjection) {
    auto* commandList = DirectXCommon::GetInstance()->GetCommandList();

    // worldTransform_ から行列を生成してからViewProjectionと合成
    Matrix4x4 worldMatrix = MakeAffineMatrix(
        worldTransform_.scale,
        worldTransform_.rotate,
        worldTransform_.translate
    );
    *wvpData_ = Multiply(worldMatrix, viewProjection);

    commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

    material_.Bind(commandList);
    mesh_.Draw(commandList);
}
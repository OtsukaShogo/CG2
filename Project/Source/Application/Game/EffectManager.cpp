#include "EffectManager.h"
#include "FeedbackSpec.h"
#include "../../Engine/3D/CameraManager.h"
#include "../../Engine/Core/Framework.h"

using namespace Engine;

namespace Shooter {

namespace {

// 同時に存在できる粒の最大数。撃破の爆発が重なっても、足りる量
constexpr uint32_t kParticleCapacity = 2048;

} // namespace

void EffectManager::Initialize(Framework* framework) {
	AssetFactory* assetFactory = framework->GetAssetFactory();
	DirectXCommon* dx = framework->GetDirectX();

	quad_ = assetFactory->CreateModelData("Resource", "plane.obj");
	texture_ = assetFactory->LoadTexture("Resource/circle.png");

	particles_.Initialize(dx->GetDevice(), dx->GetSrvDescriptorHeap(), dx->GetDescriptorSizeSRV(), kParticleCapacity);
	particles_.SetModelData(quad_);
	particles_.SetTextureHandle(texture_.gpuHandle);
}

void EffectManager::Handle(const FeedbackEvent& event) {
	const FeedbackSpec spec = GetFeedbackSpec(event.type);
	if (spec.effect == EffectId::kNone) {
		return;
	}
	particles_.EmitBurst(MakeBurst(spec.effect, event.direction), event.position);
}

void EffectManager::Update(Framework* framework, float deltaTime) {
	CameraManager* cameraMgr = framework->GetCameraManager();
	particles_.Update(cameraMgr->GetViewProjection(), cameraMgr->GetRotateMatrix(), deltaTime, framework->GetDirectX()->GetFrameContext()->frameIndex);
}

void EffectManager::Draw(Framework* framework, ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers, D3D12_GPU_VIRTUAL_ADDRESS lightAddress) {
	// 火花や爆発は、重なるほど明るくなる加算ブレンドで描く
	framework->GetPipelineStateManager()->SetPipeline(commandList, PipelineStateManager::kParticle, BlendMode::kAdd);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kDirectionalLight), lightAddress);
	particles_.Draw(commandList, constantBuffers);
}

} // namespace Shooter

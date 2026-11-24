#include "FadeOverlay.h"
#include "ShooterConfig.h"
#include "../../Engine/Core/WinApp.h"

using namespace Engine;

namespace Shooter {

void FadeOverlay::Initialize(AssetFactory* assetFactory) {
	sprite_ = assetFactory->CreateSprite(static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight));
	sprite_->SetTextureHandle(assetFactory->GetWhiteTexture().gpuHandle);
	sprite_->SetColor(Vector4{ 0.0f, 0.0f, 0.0f, 1.0f });
	fade_.StartFadeIn(Config::kFadeInTime); // 真っ暗な状態から、すぐ明るくし始める（最初の1フレームが、一瞬明るく見えないように）
}


void FadeOverlay::StartFadeOut() {
	fade_.StartFadeOut(Config::kFadeOutTime);
}

void FadeOverlay::Update(float deltaTime) {
	fade_.Update(deltaTime);
}

void FadeOverlay::Draw(const Matrix4x4& spriteViewProjection) {
	if (fade_.GetAlpha() <= 0.0f) {
		return;
	}
	sprite_->SetColor(Vector4{ 0.0f, 0.0f, 0.0f, fade_.GetAlpha() });
	sprite_->Draw(spriteViewProjection);
}

} // namespace Shooter

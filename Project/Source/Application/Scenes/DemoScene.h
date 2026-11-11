#pragma once
#include <memory>

#include "../../Engine/Scene/Scene.h"
#include "../../Engine/Core/PipelineStateManager.h"
#include "../../Engine/Graphics/Light.h"
#include "../../Engine/Graphics/TextureManager.h"
#include "../../Engine/3D/ParticleSystem.h"
#include "../../Engine/3D/Wind.h"
#include "../../Engine/2D/Sprite.h"
#include "../../Engine/Audio/AudioManager.h"
#include "../../Engine/Math/WorldTransform.h"

/// <summary>
/// 炎パーティクルのデモシーン（ゲーム本体ができるまでの動作確認用）
/// </summary>
class DemoScene : public Engine::Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	void Finalize() override;

private:
	Engine::TextureHandle tex_;
	Engine::TextureHandle particleTex_;
	Engine::SoundData soundData1_{};

	std::shared_ptr<Engine::ModelData> particleModelData_;
	std::unique_ptr<Engine::Sprite> sprite_;
	std::unique_ptr<Engine::ParticleSystem> particle_;
	std::unique_ptr<Engine::Light> light_;
	Engine::Wind wind_;

	Engine::WorldTransform uvTransformSprite_{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	int currentBlendMode_ = static_cast<int>(Engine::BlendMode::kAdd); // 炎は加算ブレンド
	bool isParticleMoving_ = true; // 起動直後から燃やす
};

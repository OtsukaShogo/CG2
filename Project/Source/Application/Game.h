#pragma once
#include <memory>

#include "../Engine/Core/Framework.h"
#include "../Engine/Graphics/Light.h"
#include "../Engine/Graphics/TextureManager.h"
#include "../Engine/3D/ParticleSystem.h"
#include "../Engine/3D/Wind.h"
#include "../Engine/2D/Sprite.h"
#include "../Engine/Audio/AudioManager.h"
#include "../Engine/Math/WorldTransform.h"

/// <summary>
/// ゲーム本体。Engine::Framework を継承し、ゲーム固有の初期化・更新・描画・終了処理を実装する
/// </summary>
class Game : public Engine::Framework {
protected:
	void OnInitialize() override;
	void OnUpdate() override;
	void OnDraw(ID3D12GraphicsCommandList* commandList) override;
	void OnFinalize() override;

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

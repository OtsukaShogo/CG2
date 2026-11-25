#pragma once
#include <memory>

#include "../../Engine/Scene/Scene.h"
#include "../../Engine/Core/PipelineStateManager.h"
#include "../../Engine/Graphics/Light.h"
#include "../../Engine/Graphics/TextureManager.h"
#include "../../Engine/3D/Model.h"
#include "../../Engine/3D/ModelData.h"
#include "../../Engine/2D/Sprite.h"
#include "../../Engine/Audio/AudioManager.h"
#include "../../Engine/Math/WorldTransform.h"

/// <summary>
/// 動作確認用のデモシーン（現在は球を描画して鏡面反射を確認する）
/// </summary>
class DemoScene : public Engine::Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	void Finalize() override;

private:
	Engine::TextureHandle tex_;
	Engine::TextureHandle sphereTex_;
	Engine::Sound soundData1_;                              // 読み込んだ音声（手放せば自動で解放される）
	Engine::VoiceId voice1_ = Engine::kInvalidVoiceId;      // 再生中の音声の識別子

	std::shared_ptr<Engine::ModelData> sphereModelData_;
	std::unique_ptr<Engine::Model> sphere_;
	std::unique_ptr<Engine::Sprite> sprite_;
	std::unique_ptr<Engine::Light> light_;

	Engine::WorldTransform uvTransformSprite_{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	int currentBlendMode_ = static_cast<int>(Engine::BlendMode::kNormal); // スプライト描画用

	Engine::Vector3 followTarget_ = { 0.0f, 0.0f, 0.0f }; // 追従カメラの確認用ターゲット
};

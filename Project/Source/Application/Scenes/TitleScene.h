#pragma once
#include <memory>
#include "../../Engine/Scene/Scene.h"
#include "../../Engine/2D/Sprite.h"
#include "../Game/FadeOverlay.h"

/// <summary>
/// タイトル画面。決定の操作で、ゲームを始める（文字表示ができるまでは、色のついた板で仮に表示する）
/// </summary>
class TitleScene : public Engine::Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	void Finalize() override;

private:
	Shooter::FadeOverlay fade_;
	std::unique_ptr<Engine::Sprite> background_;
	std::unique_ptr<Engine::Sprite> logoBar_;
	std::unique_ptr<Engine::Sprite> logoPlayer_;
	std::unique_ptr<Engine::Sprite> logoNose_;
	std::unique_ptr<Engine::Sprite> logoEnemies_[3];
	std::unique_ptr<Engine::Sprite> startPrompt_;   // 「決定で始める」の、仮の表示（脈打つ緑の板）

	float elapsed_ = 0.0f;       // シーンが始まってからの時間
	bool leaving_ = false;       // ゲームへ進む操作をして、暗転に入ったか
};

#pragma once
#include <memory>
#include <vector>
#include "../../Engine/Scene/Scene.h"
#include "../../Engine/2D/Sprite.h"
#include "../Game/FadeOverlay.h"

/// <summary>
/// ゲームオーバー画面。結果（撃破数・生存時間）を表示し、決定でもう一度遊ぶ、戻るでタイトルへ。
/// 文字表示ができるまでは、色のついた板で仮に表示する（撃破数は、赤い四角の数、生存時間は、緑の棒の長さ）
/// </summary>
class GameOverScene : public Engine::Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	void Finalize() override;

private:
	/// <summary>
	/// 移る先を決めて、暗転に入る
	/// </summary>
	void Leave(const char* nextScene);

private:
	Shooter::FadeOverlay fade_;
	std::unique_ptr<Engine::Sprite> background_;
	std::unique_ptr<Engine::Sprite> panel_;
	std::unique_ptr<Engine::Sprite> newBestBar_;
	std::vector<std::unique_ptr<Engine::Sprite>> killMarks_;  // 撃破数ぶんの、赤い四角
	std::unique_ptr<Engine::Sprite> timeBack_;
	std::unique_ptr<Engine::Sprite> timeBar_;
	std::unique_ptr<Engine::Sprite> retryPrompt_;   // 「もう一度」（仮）: 脈打つ緑の板
	std::unique_ptr<Engine::Sprite> titlePrompt_;   // 「タイトルへ」（仮）: 灰色の板

	float elapsed_ = 0.0f;
	const char* nextScene_ = nullptr; // 暗転が終わったら移る先（nullptr の間は、まだ決まっていない）
};

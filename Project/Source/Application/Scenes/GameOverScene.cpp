#include "GameOverScene.h"

#include <algorithm>
#include <cmath>

#include "../../Engine/Core/Framework.h"
#include "../../Engine/Core/WinApp.h"
#include "../../Engine/3D/CameraManager.h"
#include "../../Engine/Input/Input.h"
#include "../Game/GameAudio.h"
#include "../Game/GameInput.h"
#include "../Game/GameSession.h"
#include "../Game/ShooterConfig.h"

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

using namespace Engine;

namespace {

constexpr int kMaxKillMarks = 60;      // 表示する、赤い四角の最大数
constexpr int kMarksPerRow = 12;
constexpr float kMarkSize = 24.0f;
constexpr float kMarkGap = 8.0f;
constexpr float kTimeBarMax = 560.0f;  // 生存時間の棒の、最大の長さ（ピクセル）
constexpr float kTimeBarSeconds = 120.0f; // この秒数で、棒が最大の長さになる

} // namespace

void GameOverScene::Initialize() {
	AssetFactory* assetFactory = GetFramework()->GetAssetFactory();
	const float w = static_cast<float>(WinApp::kClientWidth);
	const float h = static_cast<float>(WinApp::kClientHeight);
	const Shooter::GameSession& session = Shooter::GameSession::Get();
	const Shooter::GameResult& result = session.GetLast();

	background_ = Shooter::CreateColorBox(assetFactory, 0.0f, 0.0f, w, h, Vector4{ 0.13f, 0.04f, 0.05f, 1.0f });
	panel_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 340.0f, 90.0f, 680.0f, 540.0f, Vector4{ 0.04f, 0.02f, 0.03f, 0.75f });

	// ベストを更新したら、金色の帯を出す
	newBestBar_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 300.0f, 120.0f, 600.0f, 16.0f, Vector4{ 1.0f, 0.85f, 0.2f, 1.0f });

	// 撃破数: 1体につき、赤い四角を1つ（多すぎる場合は、上限まで）
	const int marks = (std::min)(result.kills, kMaxKillMarks);
	const float marksLeft = w * 0.5f - 300.0f;
	for (int i = 0; i < marks; ++i) {
		const float x = marksLeft + (i % kMarksPerRow) * (kMarkSize + kMarkGap);
		const float y = 170.0f + (i / kMarksPerRow) * (kMarkSize + kMarkGap);
		killMarks_.push_back(Shooter::CreateColorBox(assetFactory, x, y, kMarkSize, kMarkSize, Vector4{ 0.9f, 0.25f, 0.25f, 1.0f }));
	}

	// 生存時間: 緑の棒の長さ（背景の枠の中に、時間の割合ぶん）
	const float ratio = (std::clamp)(result.survivalTime / kTimeBarSeconds, 0.0f, 1.0f);
	timeBack_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 300.0f, 440.0f, kTimeBarMax + 8.0f, 30.0f, Vector4{ 0.2f, 0.2f, 0.22f, 1.0f });
	timeBar_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 296.0f, 444.0f, (std::max)(2.0f, kTimeBarMax * ratio), 22.0f, Vector4{ 0.3f, 0.85f, 0.45f, 1.0f });

	// 「もう一度」と「タイトルへ」（仮）
	retryPrompt_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 300.0f, 530.0f, 280.0f, 54.0f, Vector4{ 0.3f, 0.9f, 0.4f, 1.0f });
	titlePrompt_ = Shooter::CreateColorBox(assetFactory, w * 0.5f + 40.0f, 530.0f, 260.0f, 54.0f, Vector4{ 0.45f, 0.45f, 0.5f, 1.0f });

	Shooter::GameAudio::Get().PlayBgm(Shooter::BgmId::kResult);
	fade_.Initialize(assetFactory);
}

void GameOverScene::Leave(const char* nextScene) {
	nextScene_ = nextScene;
	Shooter::GameAudio::Get().PlaySe(Shooter::SeId::kConfirm);
	fade_.StartFadeOut();
}

void GameOverScene::Update() {
	Framework* framework = GetFramework();
	const float deltaTime = framework->GetDeltaTime();
	elapsed_ += deltaTime;
	const Shooter::GameSession& session = Shooter::GameSession::Get();

#ifdef USE_IMGUI
	ImGui::Begin("Game Over");
	ImGui::Text("Kills: %d   Time: %.1f s%s", session.GetLast().kills, session.GetLast().survivalTime, session.IsLastNewBest() ? "   NEW BEST!" : "");
	ImGui::Text("Best:  %d kills   %.1f s", session.GetBest().kills, session.GetBest().survivalTime);
	ImGui::Text("Enter / Space / Click / Start / A : Retry");
	ImGui::Text("Esc / Backspace / B : Back to title");
	ImGui::End();
#endif

	// 決定でもう一度、戻るでタイトルへ（開始直後は受け付けない。暗転に入ったら、二重に要求しない）
	if (!nextScene_ && elapsed_ > Shooter::Config::kSceneInputLockTime) {
		Input* input = framework->GetInput();
		if (Shooter::IsConfirmPressed(input)) {
			Leave("Game");
		} else if (Shooter::IsBackPressed(input)) {
			Leave("Title");
		}
	}
	fade_.Update(deltaTime);
	if (nextScene_ && fade_.IsFadeOutFinished()) {
		ChangeScene(nextScene_);
	}

	const float pulse = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(elapsed_ * 4.0f));
	retryPrompt_->SetColor(Vector4{ 0.3f * pulse, 0.9f * pulse, 0.4f * pulse, 1.0f });
}

void GameOverScene::Draw(ID3D12GraphicsCommandList* commandList) {
	Framework* framework = GetFramework();
	framework->GetPipelineStateManager()->SetPipeline(commandList, PipelineStateManager::kSprite, BlendMode::kNormal);
	const Matrix4x4& spriteViewProjection = framework->GetCameraManager()->GetSpriteViewProjection();

	background_->Draw(spriteViewProjection);
	panel_->Draw(spriteViewProjection);
	if (Shooter::GameSession::Get().IsLastNewBest()) {
		newBestBar_->Draw(spriteViewProjection);
	}
	for (auto& mark : killMarks_) {
		mark->Draw(spriteViewProjection);
	}
	timeBack_->Draw(spriteViewProjection);
	timeBar_->Draw(spriteViewProjection);
	retryPrompt_->Draw(spriteViewProjection);
	titlePrompt_->Draw(spriteViewProjection);
	fade_.Draw(spriteViewProjection);
}

void GameOverScene::Finalize() {
	killMarks_.clear();
}

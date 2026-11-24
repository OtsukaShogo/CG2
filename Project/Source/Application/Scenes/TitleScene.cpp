#include "TitleScene.h"

#include <cmath>

#include "../../Engine/Core/Framework.h"
#include "../../Engine/Core/WinApp.h"
#include "../../Engine/3D/CameraManager.h"
#include "../../Engine/Input/Input.h"
#include "../Game/GameInput.h"
#include "../Game/ShooterConfig.h"

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

using namespace Engine;

void TitleScene::Initialize() {
	AssetFactory* assetFactory = GetFramework()->GetAssetFactory();
	const float w = static_cast<float>(WinApp::kClientWidth);
	const float h = static_cast<float>(WinApp::kClientHeight);

	// 文字がまだ使えないので、ロゴとボタンは、色のついた板で仮に表示する
	background_ = Shooter::CreateColorBox(assetFactory, 0.0f, 0.0f, w, h, Vector4{ 0.06f, 0.08f, 0.14f, 1.0f });

	// ロゴ（仮）: 青い帯、その上に、自機（青い柱と黄色い鼻）と、敵（赤い箱）が並ぶ
	logoBar_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 240.0f, 330.0f, 480.0f, 18.0f, Vector4{ 0.3f, 0.6f, 1.0f, 1.0f });
	logoPlayer_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 22.0f, 190.0f, 44.0f, 120.0f, Vector4{ 0.3f, 0.6f, 1.0f, 1.0f });
	logoNose_ = Shooter::CreateColorBox(assetFactory, w * 0.5f + 22.0f, 225.0f, 40.0f, 18.0f, Vector4{ 1.0f, 0.85f, 0.2f, 1.0f });
	const float enemyX[3] = { w * 0.5f + 130.0f, w * 0.5f + 210.0f, w * 0.5f + 175.0f };
	const float enemyY[3] = { 235.0f, 255.0f, 180.0f };
	for (int i = 0; i < 3; ++i) {
		logoEnemies_[i] = Shooter::CreateColorBox(assetFactory, enemyX[i], enemyY[i], 52.0f, 70.0f, Vector4{ 0.9f, 0.25f, 0.25f, 1.0f });
	}

	// 「決定で始める」（仮）: 脈打つ緑の板
	startPrompt_ = Shooter::CreateColorBox(assetFactory, w * 0.5f - 160.0f, 470.0f, 320.0f, 54.0f, Vector4{ 0.3f, 0.9f, 0.4f, 1.0f });

	fade_.Initialize(assetFactory);
}

void TitleScene::Update() {
	Framework* framework = GetFramework();
	const float deltaTime = framework->GetDeltaTime();
	elapsed_ += deltaTime;

#ifdef USE_IMGUI
	ImGui::Begin("Title");
	ImGui::Text("Enter / Space / Click / Start / A : Start game");
	if (ImGui::Button("Open Demo Scene")) {
		ChangeScene("Demo");
	}
	ImGui::End();
#endif

	// 「決定」で、ゲームへ（シーンの開始直後は、前のシーンのクリックが効かないよう、受け付けない）
	if (!leaving_ && elapsed_ > Shooter::Config::kSceneInputLockTime && Shooter::IsConfirmPressed(framework->GetInput())) {
		leaving_ = true;
		fade_.StartFadeOut();
	}
	fade_.Update(deltaTime);
	if (leaving_ && fade_.IsFadeOutFinished()) {
		ChangeScene("Game");
	}

	// 「決定で始める」の板を、ゆっくり脈打たせる
	const float pulse = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(elapsed_ * 4.0f));
	startPrompt_->SetColor(Vector4{ 0.3f * pulse, 0.9f * pulse, 0.4f * pulse, 1.0f });
}

void TitleScene::Draw(ID3D12GraphicsCommandList* commandList) {
	Framework* framework = GetFramework();
	framework->GetPipelineStateManager()->SetPipeline(commandList, PipelineStateManager::kSprite, BlendMode::kNormal);
	const Matrix4x4& spriteViewProjection = framework->GetCameraManager()->GetSpriteViewProjection();

	background_->Draw(spriteViewProjection);
	logoBar_->Draw(spriteViewProjection);
	logoPlayer_->Draw(spriteViewProjection);
	logoNose_->Draw(spriteViewProjection);
	for (auto& enemy : logoEnemies_) {
		enemy->Draw(spriteViewProjection);
	}
	startPrompt_->Draw(spriteViewProjection);
	fade_.Draw(spriteViewProjection);
}

void TitleScene::Finalize() {
}

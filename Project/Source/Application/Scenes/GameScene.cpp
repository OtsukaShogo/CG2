#include "GameScene.h"

#include <algorithm>
#include <cmath>

#include "../../Engine/Core/Framework.h"
#include "../../Engine/3D/CameraManager.h"
#include "../../Engine/Core/WinApp.h"
#include "../../Engine/DebugTools/DebugDraw.h"
#include "../../Engine/Input/Input.h"
#include "../Game/PlayerMath.h"
#include "../Game/ShooterConfig.h"

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

using namespace Engine;

void GameScene::Initialize() {
	Framework* framework = GetFramework();
	AssetFactory* assetFactory = framework->GetAssetFactory();

	arena_.Initialize(assetFactory);
	player_.Initialize(assetFactory, Vector3{ 0.0f, 0.0f, 0.0f });

	// 斜めから見るので、光は、手前上から奥へ向かう向きにして、箱の側面にも陰影が出るようにする
	light_.GetDirectionalLight()->direction = Normalize(Vector3{ -0.4f, -1.0f, 0.6f });

	SetupCamera();
}

void GameScene::SetupCamera() {
	CameraManager* cameraMgr = GetFramework()->GetCameraManager();
	Camera* camera = cameraMgr->GetCamera();
	CameraController* controller = cameraMgr->GetController();

	savedProjection_ = camera->GetProjectionType();
	savedOrthoHeight_ = camera->GetOrthoHeight();
	savedTranslate_ = camera->GetTranslate();
	camera->SetProjectionType(ProjectionType::kOrthographic);
	camera->GetOrthoHeight() = Shooter::Config::kCameraOrthoHeight;

	controller->GetPitch() = Shooter::Config::kCameraPitch;
	controller->GetYaw() = 0.0f;
	controller->GetDistance() = Shooter::Config::kCameraDistance;
	controller->GetFollowSpeed() = Shooter::Config::kCameraFollowSpeed;
	controller->GetOffset() = Vector3{ 0.0f, 0.0f, 0.0f };

	// カメラが追従する範囲: 画面がアリーナの外（壁の向こう）を映しすぎないよう、見える範囲の分だけ、内側に絞る
	const float aspect = static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight);
	const float viewHalfWidth = Shooter::Config::kCameraOrthoHeight * aspect * 0.5f;
	const float viewHalfDepth = Shooter::Config::kCameraOrthoHeight * 0.5f / std::sin(Shooter::Config::kCameraPitch); // 斜めなので、地面上では縦に伸びる
	const Vector2 half = arena_.GetHalfExtent();
	const float boundX = (std::max)(0.0f, half.x + Shooter::Config::kWallThickness - viewHalfWidth); // 壁の厚み分まで映す
	const float boundZ = (std::max)(0.0f, half.y + Shooter::Config::kWallThickness - viewHalfDepth);
	controller->SetBounds(Vector2{ -boundX, -boundZ }, Vector2{ boundX, boundZ });

	controller->SetTarget(&player_.GetPosition());
	controller->SetEnabled(true);
	controller->Snap();
}

Vector3 GameScene::ReadMoveDirection() const {
	Framework* framework = GetFramework();
	Input* input = framework->GetInput();
	const Keyboard& keyboard = input->GetKeyboard();

	Shooter::MoveInput move;
	move.left = keyboard.PushKey(DIK_A) || keyboard.PushKey(DIK_LEFT);
	move.right = keyboard.PushKey(DIK_D) || keyboard.PushKey(DIK_RIGHT);
	move.up = keyboard.PushKey(DIK_W) || keyboard.PushKey(DIK_UP);
	move.down = keyboard.PushKey(DIK_S) || keyboard.PushKey(DIK_DOWN);

	const int padIndex = input->GetFirstConnectedGamepadIndex();
	if (padIndex >= 0) {
		move.stick = input->GetGamepad(padIndex).GetLeftStick();
	}

	// 画面基準の入力を、カメラが地面上で向いている向きに合わせる（デバッグカメラではなく、ゲームのカメラ基準）
	const Matrix4x4& rotation = framework->GetCameraManager()->GetCamera()->GetRotateMatrix();
	const Vector2 screenRight = Shooter::ToGroundDirection(Vector3{ rotation.m[0][0], rotation.m[0][1], rotation.m[0][2] });
	const Vector2 screenUp = Shooter::ToGroundDirection(Vector3{ rotation.m[1][0], rotation.m[1][1], rotation.m[1][2] });

	return Shooter::MoveInputToWorld(Shooter::CombineMoveInput(move), screenRight, screenUp);
}

void GameScene::Update() {
	Framework* framework = GetFramework();
	const float deltaTime = framework->GetDeltaTime();
	CameraManager* cameraMgr = framework->GetCameraManager();

#ifdef USE_IMGUI
	ImGui::Begin("Shooter");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Text("Move: WASD / Arrow keys / Left stick");
	ImGui::Checkbox("Show Colliders", &showColliders_);
	if (ImGui::Button("Open Demo Scene")) {
		ChangeScene("Demo");
	}
	ImGui::Separator();
	cameraMgr->DrawImGui();
	cameraMgr->GetController()->DrawImGui(); // 見下ろす角度・距離・追従の速さを、動かしながら調整できる
	ImGui::End();
#endif

	player_.Update(deltaTime, ReadMoveDirection(), arena_);

	// 追従カメラは、自機の更新後に動かす
	cameraMgr->GetController()->Update(deltaTime);

	if (showColliders_) {
		DebugDraw* debugDraw = framework->GetDebugDraw();
		arena_.DrawDebug(*debugDraw);
		player_.DrawDebug(*debugDraw);
	}
}

void GameScene::Draw(ID3D12GraphicsCommandList* commandList) {
	Framework* framework = GetFramework();
	const Matrix4x4& viewProjection = framework->GetCameraManager()->GetViewProjection();

	// 不透明なモデルを、ブレンドなしで描く
	framework->GetPipelineStateManager()->SetPipeline(commandList, PipelineStateManager::kObject3D, BlendMode::kNone);
	ConstantBufferAllocator& constantBuffers = *framework->GetDirectX()->GetFrameContext()->constantBuffers;
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(PipelineStateManager::RootParameter::kDirectionalLight), light_.GetDirectionalLightAddress(constantBuffers));

	arena_.Draw(viewProjection);
	player_.Draw(viewProjection);
}

void GameScene::Finalize() {
	// カメラは、シーンをまたいで残るので、このシーンのメンバを指したまま・設定を変えたままにならないようにする
	CameraManager* cameraMgr = GetFramework()->GetCameraManager();
	CameraController* controller = cameraMgr->GetController();
	controller->SetTarget(nullptr);
	controller->SetEnabled(false);
	controller->ClearBounds();
	Camera* camera = cameraMgr->GetCamera();
	camera->SetProjectionType(savedProjection_);
	camera->GetOrthoHeight() = savedOrthoHeight_;
	camera->GetTranslate() = savedTranslate_;
	camera->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
	camera->Update();
}

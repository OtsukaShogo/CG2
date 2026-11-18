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
	bullets_.Initialize(assetFactory, Shooter::Config::kBulletPoolCapacity);

	// 照準: 地面に置く、薄い円盤（光を受けず、常に見えるようにする）
	reticle_ = assetFactory->CreateModelInstance(assetFactory->CreatePrimitiveModelData(PrimitiveType::kCylinder));
	reticle_->GetScale() = Vector3{ Shooter::Config::kReticleRadius * 2.0f, 0.02f, Shooter::Config::kReticleRadius * 2.0f };
	reticle_->SetColor(Vector4{ 1.0f, 0.35f, 0.3f, 1.0f });
	reticle_->GetMaterial().SetEnableLighting(false);

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

void GameScene::GetScreenBasis(Vector2* outRight, Vector2* outUp) const {
	// 画面の右・上が、地面上でどちらを向いているか（デバッグカメラではなく、ゲームのカメラ基準）
	const Matrix4x4& rotation = GetFramework()->GetCameraManager()->GetCamera()->GetRotateMatrix();
	*outRight = Shooter::ToGroundDirection(Vector3{ rotation.m[0][0], rotation.m[0][1], rotation.m[0][2] });
	*outUp = Shooter::ToGroundDirection(Vector3{ rotation.m[1][0], rotation.m[1][1], rotation.m[1][2] });
}

Vector3 GameScene::ReadMoveDirection(const Vector2& screenRight, const Vector2& screenUp) const {
	Input* input = GetFramework()->GetInput();
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

	return Shooter::MoveInputToWorld(Shooter::CombineMoveInput(move), screenRight, screenUp);
}

void GameScene::UpdateAim(const Vector2& screenRight, const Vector2& screenUp) {
	Framework* framework = GetFramework();
	Input* input = framework->GetInput();
	const Mouse& mouse = input->GetMouse();
	const Camera* camera = framework->GetCameraManager()->GetCamera();

	const int padIndex = input->GetFirstConnectedGamepadIndex();
	const Vector2 rightStick = padIndex >= 0 ? input->GetGamepad(padIndex).GetRightStick() : Vector2{ 0.0f, 0.0f };
	const bool stickActive = Shooter::IsStickActive(rightStick, Shooter::Config::kAimStickThreshold);
	const bool mouseActive = mouse.GetDeltaX() != 0 || mouse.GetDeltaY() != 0 || mouse.PushButton(MouseButton::kLeft);

	aimSource_ = Shooter::ChooseAimSource(aimSource_, mouseActive, stickActive);

	const Vector3& playerPosition = player_.GetPosition();
	reticleVisible_ = false;

	if (aimSource_ == Shooter::AimSource::kMouse) {
		// マウス: カーソルを通る視線と、弾の高さの水平面との交点を狙う
		aiming_ = true;
		Vector3 point;
		if (mouse.IsInsideWindow() && camera->ScreenToWorldOnPlane(mouse.GetPosition(), Shooter::Config::kBulletHeight, &point)) {
			aimDirection_ = Shooter::AimDirectionFromPoint(playerPosition, point, aimDirection_);
			aimPoint_ = Vector3{ point.x, 0.0f, point.z };
			reticleVisible_ = true;
		}
	} else if (stickActive) {
		// ゲームパッド: 右スティックを倒した向きへ（画面基準 → カメラの向きに合わせる）
		aiming_ = true;
		const Vector3 world = Shooter::MoveInputToWorld(rightStick, screenRight, screenUp);
		aimDirection_ = Normalize(world);
		aimPoint_ = playerPosition + aimDirection_ * Shooter::Config::kPadReticleDistance;
		aimPoint_.y = 0.0f;
		reticleVisible_ = true;
	} else {
		aiming_ = false; // スティックを離している間は、狙っていない（直近の向きは覚えておく）
	}
}

bool GameScene::IsFireHeld() const {
	Input* input = GetFramework()->GetInput();
	bool held = input->GetMouse().PushButton(MouseButton::kLeft);

	const int padIndex = input->GetFirstConnectedGamepadIndex();
	if (padIndex >= 0) {
		const Gamepad& pad = input->GetGamepad(padIndex);
		// 右スティックは、狙う向きを決めるだけで、倒しても撃たない（撃つのは RT / RB）
		held = held || pad.PushRightTrigger() || pad.PushButton(GamepadButton::kRightShoulder);
	}
	return held;
}

void GameScene::Update() {
	Framework* framework = GetFramework();
	const float deltaTime = framework->GetDeltaTime();
	CameraManager* cameraMgr = framework->GetCameraManager();

#ifdef USE_IMGUI
	ImGui::Begin("Shooter");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Text("Move: WASD / Arrow keys / Left stick");
	ImGui::Text("Aim: Mouse / Right stick   Fire: Left click / RT / RB (stick only aims)");
	ImGui::Text("Aim source: %s   Bullets: %d", aimSource_ == Shooter::AimSource::kMouse ? "Mouse" : "Gamepad", static_cast<int>(bullets_.GetActiveCount()));
	ImGui::Checkbox("Show Colliders", &showColliders_);
	if (ImGui::Button("Open Demo Scene")) {
		ChangeScene("Demo");
	}
	ImGui::Separator();
	cameraMgr->DrawImGui();
	cameraMgr->GetController()->DrawImGui(); // 見下ろす角度・距離・追従の速さを、動かしながら調整できる
	ImGui::End();
#endif

	Vector2 screenRight, screenUp;
	GetScreenBasis(&screenRight, &screenUp);

	// 照準 → 自機の移動と向き → 発射 の順（照準は、直前のカメラの位置で決まる）
	UpdateAim(screenRight, screenUp);
	player_.Update(deltaTime, ReadMoveDirection(screenRight, screenUp), aiming_ ? aimDirection_ : Vector3{ 0.0f, 0.0f, 0.0f }, arena_);

	const int shots = Shooter::ConsumeShots(fireCooldown_, deltaTime, IsFireHeld(), Shooter::Config::kFireInterval);
	if (shots > 0) {
		// 撃つ向き: 狙っていればその向き。ゲームパッドで右スティックを離していれば、自機が向いている向き
		const Vector3 fireDirection = aiming_ ? aimDirection_ : Vector3{ std::sin(player_.GetYaw()), 0.0f, std::cos(player_.GetYaw()) };
		// 弾は、自機の位置から、少し前に出た高さ（弾の高さ）で発射する
		const Vector3 muzzle = player_.GetPosition() + fireDirection * Shooter::Config::kMuzzleDistance + Vector3{ 0.0f, Shooter::Config::kBulletHeight, 0.0f };
		for (int i = 0; i < shots; ++i) {
			bullets_.Fire(muzzle, fireDirection);
		}
	}
	bullets_.Update(deltaTime, arena_.GetWalls());

	// 照準の円盤の位置
	reticle_->GetTranslate() = Vector3{ aimPoint_.x, 0.03f, aimPoint_.z };

	// 追従カメラは、自機の更新後に動かす
	cameraMgr->GetController()->Update(deltaTime);

	if (showColliders_) {
		DebugDraw* debugDraw = framework->GetDebugDraw();
		arena_.DrawDebug(*debugDraw);
		player_.DrawDebug(*debugDraw);
		bullets_.DrawDebug(*debugDraw);
		if (reticleVisible_) {
			// 自機から照準までの線（弾が進む向き）
			const Vector3 from = player_.GetPosition() + Vector3{ 0.0f, Shooter::Config::kBulletHeight, 0.0f };
			debugDraw->Line(from, Vector3{ aimPoint_.x, Shooter::Config::kBulletHeight, aimPoint_.z }, Vector4{ 1.0f, 0.4f, 0.3f, 0.6f });
		}
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
	if (reticleVisible_) {
		reticle_->Draw(viewProjection);
	}
	player_.Draw(viewProjection);
	bullets_.Draw(viewProjection);
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

	bullets_.Clear();
}

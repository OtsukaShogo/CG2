#include "CameraManager.h"
#include "../Input/Input.h"

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

namespace Engine {

CameraManager* CameraManager::GetInstance() {
	static CameraManager instance;
	return &instance;
}

void CameraManager::Initialize() {
	camera_ = std::make_unique<Camera>();
	camera_->Update();

	controller_ = std::make_unique<CameraController>();
	controller_->Initialize(camera_.get());

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
}

void CameraManager::Update() {
	Input* input = Input::GetInstance();

	// F1キーで通常カメラ/デバッグカメラを切り替える
	if (input->GetKeyboard().TriggerKey(DIK_F1)) {
		isDebugCamera_ = !isDebugCamera_;
	}

	// 有効な方のカメラだけを更新する
	if (isDebugCamera_) {
		debugCamera_->Update();
	} else {
		camera_->Update();
	}
}

const Camera* CameraManager::GetActiveCamera() const {
	if (isDebugCamera_) {
		return &debugCamera_->GetCamera();
	}
	return camera_.get();
}

const Matrix4x4& CameraManager::GetViewProjection() const {
	if (isDebugCamera_) {
		return debugCamera_->GetViewProjection();
	}
	return camera_->GetViewProjection();
}

const Matrix4x4& CameraManager::GetRotateMatrix() const {
	if (isDebugCamera_) {
		return debugCamera_->GetRotateMatrix();
	}
	return camera_->GetRotateMatrix();
}

const Matrix4x4& CameraManager::GetSpriteViewProjection() const {
	return camera_->GetSpriteViewProjection();
}

#ifdef USE_IMGUI
void CameraManager::DrawImGui() {
	ImGui::Checkbox("Debug Camera (F1)", &isDebugCamera_);
	if (!isDebugCamera_) {
		controller_->DrawImGui();

		if (controller_->IsEnabled()) {
			// 位置・回転はコントローラが毎フレーム決めるので手動編集は出さない（無効に戻したとき表示がずれないよう値だけ同期）
			cameraRotate_ = { controller_->GetPitch(), controller_->GetYaw(), 0.0f };
		} else {
			ImGui::DragFloat3("Camera Translate", &camera_->GetTranslate().x, 0.1f);
			if (ImGui::DragFloat3("Camera Rotate", &cameraRotate_.x, 0.01f)) {
				camera_->SetRotation(cameraRotate_);
			}
		}
		int projection = static_cast<int>(camera_->GetProjectionType());
		if (ImGui::Combo("Camera Projection", &projection, "Perspective\0Orthographic\0")) {
			camera_->SetProjectionType(static_cast<ProjectionType>(projection));
		}
		if (camera_->GetProjectionType() == ProjectionType::kOrthographic) {
			ImGui::DragFloat("Camera Ortho Height", &camera_->GetOrthoHeight(), 0.1f, 0.1f, 200.0f);
		} else {
			ImGui::SliderFloat("Camera FovY", &camera_->GetFovY(), 0.1f, 3.0f);
		}
		ImGui::DragFloat("Camera NearZ", &camera_->GetNearZ(), 0.01f, 0.01f, camera_->GetFarZ() - 0.01f);
		ImGui::DragFloat("Camera FarZ", &camera_->GetFarZ(), 0.1f, camera_->GetNearZ() + 0.01f, 1000.0f);
	}
}
#endif

} // namespace Engine

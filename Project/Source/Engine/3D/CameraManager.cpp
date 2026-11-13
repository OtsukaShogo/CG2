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
}
#endif

} // namespace Engine

#include "CameraManager.h"
#include "Input.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

CameraManager* CameraManager::GetInstance() {
	static CameraManager instance;
	return &instance;
}

void CameraManager::Initialize() {
	camera_ = std::make_unique<Camera>();
	camera_->Update();

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
}

void CameraManager::Update() {
	Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_F1)) {
		isDebugCamera_ = !isDebugCamera_;
	}

	if (isDebugCamera_) {
		debugCamera_->Update();
	} else {
		camera_->Update();
	}
}

const Matrix4x4& CameraManager::GetViewProjection() const {
	if (isDebugCamera_) {
		return debugCamera_->GetViewProjection();
	}
	return camera_->GetViewProjection();
}

const Matrix4x4& CameraManager::GetSpriteViewProjection() const {
	return camera_->GetSpriteViewProjection();
}

#ifdef USE_IMGUI
void CameraManager::DrawImGui() {
	ImGui::Checkbox("Debug Camera (F1)", &isDebugCamera_);
	if (!isDebugCamera_) {
		ImGui::DragFloat3("Camera Translate", &camera_->GetTranslate().x, 0.1f);
	}
}
#endif

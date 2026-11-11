#include "CameraController.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

namespace Engine {

void CameraController::Initialize(Camera* camera) {
	camera_ = camera;
}

void CameraController::SetTargetPosition(const Vector3& position) {
	targetValue_ = position;
	target_ = nullptr;
}

void CameraController::SetBounds(const Vector2& min, const Vector2& max) {
	boundsMin_ = min;
	boundsMax_ = max;
	useBounds_ = true;
}

Vector3 CameraController::CalcDesiredFocus() const {
	const Vector3& base = target_ ? *target_ : targetValue_;
	Vector3 focus = { base.x + offset_.x, base.y + offset_.y, base.z + offset_.z };

	if (useBounds_) {
		focus.x = std::clamp(focus.x, boundsMin_.x, boundsMax_.x);
		focus.z = std::clamp(focus.z, boundsMin_.y, boundsMax_.y);
	}
	return focus;
}

void CameraController::ApplyToCamera() {
	assert(camera_);

	// 回転を先に反映し、その前方向ベクトルの逆向きに distance_ だけ離れた位置へカメラを置く
	camera_->SetRotation({ pitch_, yaw_, 0.0f });

	const Matrix4x4& rot = camera_->GetRotateMatrix();
	Vector3 forward = { rot.m[2][0], rot.m[2][1], rot.m[2][2] };

	camera_->GetTranslate() = {
		focusPoint_.x - forward.x * distance_,
		focusPoint_.y - forward.y * distance_,
		focusPoint_.z - forward.z * distance_,
	};

	// 直後の座標変換（ScreenToWorldOnPlane等）にも最新のカメラが使われるよう、ここで行列を更新する
	camera_->Update();
}

void CameraController::Snap() {
	if (!camera_) {
		return;
	}
	focusPoint_ = CalcDesiredFocus();
	ApplyToCamera();
}

void CameraController::Update(float deltaTime) {
	if (!enabled_ || !camera_) {
		return;
	}

	Vector3 desired = CalcDesiredFocus();

	if (followSpeed_ <= 0.0f) {
		focusPoint_ = desired;
	} else {
		// フレームレートに依存しない指数減衰による追従（1 - e^(-speed * dt) ずつ目標に近づく）
		float t = 1.0f - std::exp(-followSpeed_ * deltaTime);
		focusPoint_.x += (desired.x - focusPoint_.x) * t;
		focusPoint_.y += (desired.y - focusPoint_.y) * t;
		focusPoint_.z += (desired.z - focusPoint_.z) * t;
	}

	ApplyToCamera();
}

#ifdef USE_IMGUI
void CameraController::DrawImGui() {
	ImGui::Checkbox("Camera Controller", &enabled_);
	if (!enabled_) {
		return;
	}

	ImGui::SliderAngle("Pitch", &pitch_, 10.0f, 90.0f);
	ImGui::SliderAngle("Yaw", &yaw_, -180.0f, 180.0f);
	ImGui::DragFloat("Distance", &distance_, 0.1f, 1.0f, 90.0f);
	ImGui::DragFloat3("Focus Offset", &offset_.x, 0.1f);
	ImGui::DragFloat("Follow Speed", &followSpeed_, 0.1f, 0.0f, 50.0f);
}
#endif

} // namespace Engine

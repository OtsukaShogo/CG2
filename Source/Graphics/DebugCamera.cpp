#include "DebugCamera.h"
#include "Input.h"

namespace Engine {

void DebugCamera::Initialize() {
	camera_.Initialize();
}

void DebugCamera::Update() {
	Input* input = Input::GetInstance();
	Vector3& translate = camera_.GetTranslate();

	// 右ドラッグで回転を累積
	if (input->PushMouseButton(1)) {
		float deltaX = input->GetMouseDeltaY() * kRotateSpeed;
		float deltaY = input->GetMouseDeltaX() * kRotateSpeed;
		camera_.AddRotation(deltaX, deltaY);
	}

	// 回転行列から右ベクトルを取得してA/D移動に使用
	const Matrix4x4& rot = camera_.GetRotateMatrix();
	Vector3 right = { rot.m[0][0], rot.m[0][1], rot.m[0][2] };
	Vector3 forward = { rot.m[2][0], rot.m[2][1], rot.m[2][2] };

	// ホイールで前後移動（カメラの向き方向）
	long wheel = input->GetMouseDeltaWheel();
	if (wheel != 0) {
		float step = wheel * kZoomSpeed;
		translate.x += forward.x * step;
		translate.y += forward.y * step;
		translate.z += forward.z * step;
	}

	// WASDで左右・上下移動
	if (input->PushKey(DIK_W)) { translate.y += kMoveSpeed; }
	if (input->PushKey(DIK_S)) { translate.y -= kMoveSpeed; }
	if (input->PushKey(DIK_A)) { translate.x -= right.x * kMoveSpeed; translate.z -= right.z * kMoveSpeed; }
	if (input->PushKey(DIK_D)) { translate.x += right.x * kMoveSpeed; translate.z += right.z * kMoveSpeed; }

	camera_.Update();
}

} // namespace Engine
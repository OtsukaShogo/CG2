#include "DebugCamera.h"
#include "Input.h"

namespace Engine {

void DebugCamera::Initialize() {
	camera_.Initialize();
}

void DebugCamera::Update() {
	Input* input = Input::GetInstance();
	Vector3& translate = camera_.GetTranslate();

	bool rightButtonPushed = input->PushMouseButton(1);

	// 右ドラッグで回転を累積
	if (rightButtonPushed) {
		float deltaX = input->GetMouseDeltaY() * kRotateSpeed;
		float deltaY = input->GetMouseDeltaX() * kRotateSpeed;
		camera_.AddRotation(deltaX, deltaY);
	}

	// 回転行列から右・上・前ベクトルを取得して移動に使用
	const Matrix4x4& rot = camera_.GetRotateMatrix();
	Vector3 right = { rot.m[0][0], rot.m[0][1], rot.m[0][2] };
	Vector3 up = { rot.m[1][0], rot.m[1][1], rot.m[1][2] };
	Vector3 forward = { rot.m[2][0], rot.m[2][1], rot.m[2][2] };

	// 右クリック押し込み中はWASDで上下左右に移動（Unityのシーンビュー操作と同様）
	if (rightButtonPushed) {
		if (input->PushKey(DIK_W)) { translate.x += up.x * kMoveSpeed; translate.y += up.y * kMoveSpeed; translate.z += up.z * kMoveSpeed; }
		if (input->PushKey(DIK_S)) { translate.x -= up.x * kMoveSpeed; translate.y -= up.y * kMoveSpeed; translate.z -= up.z * kMoveSpeed; }
		if (input->PushKey(DIK_A)) { translate.x -= right.x * kMoveSpeed; translate.y -= right.y * kMoveSpeed; translate.z -= right.z * kMoveSpeed; }
		if (input->PushKey(DIK_D)) { translate.x += right.x * kMoveSpeed; translate.y += right.y * kMoveSpeed; translate.z += right.z * kMoveSpeed; }
	}

	// ホイール回転で前後移動（カメラの向き方向）
	long wheel = input->GetMouseDeltaWheel();
	if (wheel != 0) {
		float step = wheel * kZoomSpeed;
		translate.x += forward.x * step;
		translate.y += forward.y * step;
		translate.z += forward.z * step;
	}

	// ホイール押し込み（中ボタン）ドラッグで上下左右に平行移動
	if (input->PushMouseButton(2)) {
		float dx = input->GetMouseDeltaX() * kPanSpeed;
		float dy = input->GetMouseDeltaY() * kPanSpeed;
		translate.x -= right.x * dx - up.x * dy;
		translate.y -= right.y * dx - up.y * dy;
		translate.z -= right.z * dx - up.z * dy;
	}

	camera_.Update();
}

} // namespace Engine
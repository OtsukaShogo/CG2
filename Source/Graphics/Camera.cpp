#include "Camera.h"
#include "TransformMatrix.h"
#include "WinApp.h"

namespace Engine {

Camera::Camera() {
    rotateMatrix_ = MakeIdentity4x4();
}

Camera::~Camera(){}

void Camera::Initialize() {
	Update();
}

// 現在の回転行列に対して、追加の回転を左から掛けて累積させる
void Camera::AddRotation(float deltaX, float deltaY) {
    Matrix4x4 matRotDelta = MakeIdentity4x4();
    matRotDelta = Multiply(matRotDelta, MakeRotateXMatrix(deltaX));
    matRotDelta = Multiply(matRotDelta, MakeRotateYMatrix(deltaY));
    rotateMatrix_ = Multiply(matRotDelta, rotateMatrix_);
}

void Camera::Update() {
    //移動行列
    Matrix4x4 translateMatrix = MakeTranslateMatrix(translate_);

    // ビュー行列
    Matrix4x4 worldMatrix = Multiply(rotateMatrix_, translateMatrix);
    viewMatrix_ = Inverse(worldMatrix);

    // プロジェクション行列
    float aspect = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
    projMatrix_ = MakePerspectiveFovMatrix(fovY_, aspect, nearZ_, farZ_);

    viewProjection_ = Multiply(viewMatrix_, projMatrix_);

    // 2D用の正射影行列
    spriteViewProjection_ = MakeOrthographicMatrix(
        0.0f, 0.0f,
        float(WinApp::kClientWidth),
        float(WinApp::kClientHeight),
        0.0f, 100.0f
    );
}

} // namespace Engine
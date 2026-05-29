#include "Camera.h"
#include "TransformMatrix.h"
#include "WinApp.h"

Camera::Camera(){}

Camera::~Camera(){}

void Camera::Update() {
   //回転行列
    Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate_.x);
    Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate_.y);
    Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate_.z);
    Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

    //移動行列
    Matrix4x4 translateMatrix = MakeTranslateMatrix(translate_);

    // ビュー行列
    Matrix4x4 worldMatrix = Multiply(rotateMatrix, translateMatrix);
    Matrix4x4 view = Inverse(worldMatrix);

    // プロジェクション行列
    float aspect = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
    Matrix4x4 proj = MakePerspectiveFovMatrix(fovY_, aspect, nearZ_, farZ_);

    viewProjection_ = Multiply(view, proj);

    // 2D用の正射影行列
    spriteViewProjection_ = MakeOrthographicMatrix(
        0.0f, 0.0f,
        float(WinApp::kClientWidth),
        float(WinApp::kClientHeight),
        0.0f, 100.0f
    );
}
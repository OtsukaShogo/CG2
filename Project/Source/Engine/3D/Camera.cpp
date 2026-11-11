#include "Camera.h"
#include "../Math/TransformMatrix.h"
#include "../Core/WinApp.h"
#include <cmath>

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

// オイラー角から回転行列を作り直す
void Camera::SetRotation(const Vector3& rotate) {
    rotateMatrix_ = Multiply(Multiply(MakeRotateXMatrix(rotate.x), MakeRotateYMatrix(rotate.y)), MakeRotateZMatrix(rotate.z));
}

void Camera::Update() {
    //移動行列
    Matrix4x4 translateMatrix = MakeTranslateMatrix(translate_);

    // ビュー行列
    Matrix4x4 worldMatrix = Multiply(rotateMatrix_, translateMatrix);
    viewMatrix_ = Inverse(worldMatrix);

    // プロジェクション行列
    float aspect = float(WinApp::kClientWidth) / float(WinApp::kClientHeight);
    if (projectionType_ == ProjectionType::kOrthographic) {
        // 画面中心が原点、上が+Y（ワールド単位で orthoHeight_ の高さを映す）
        float halfHeight = orthoHeight_ * 0.5f;
        float halfWidth = halfHeight * aspect;
        projMatrix_ = MakeOrthographicMatrix(-halfWidth, halfHeight, halfWidth, -halfHeight, nearZ_, farZ_);
    } else {
        projMatrix_ = MakePerspectiveFovMatrix(fovY_, aspect, nearZ_, farZ_);
    }

    viewProjection_ = Multiply(viewMatrix_, projMatrix_);
    inverseViewProjection_ = Inverse(viewProjection_);

    // 2D用の正射影行列
    spriteViewProjection_ = MakeOrthographicMatrix(
        0.0f, 0.0f,
        float(WinApp::kClientWidth),
        float(WinApp::kClientHeight),
        0.0f, 100.0f
    );
}

Ray Camera::ScreenToRay(const Vector2& screenPos) const {
    // スクリーン座標 → NDC（x,yは-1〜1、yは上が+）
    float ndcX = screenPos.x / float(WinApp::kClientWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - screenPos.y / float(WinApp::kClientHeight) * 2.0f;

    // ニア面(z=0)とファー面(z=1)上の点をワールドへ戻し、その2点を結ぶ線を視線とする
    Vector3 nearPoint = Transform({ ndcX, ndcY, 0.0f }, inverseViewProjection_);
    Vector3 farPoint = Transform({ ndcX, ndcY, 1.0f }, inverseViewProjection_);

    Vector3 dir = { farPoint.x - nearPoint.x, farPoint.y - nearPoint.y, farPoint.z - nearPoint.z };
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (length > 0.0f) {
        dir = dir * (1.0f / length);
    }

    return { nearPoint, dir };
}

bool Camera::ScreenToWorldOnPlane(const Vector2& screenPos, float planeY, Vector3* outPoint) const {
    Ray ray = ScreenToRay(screenPos);

    // 視線が水平面と（ほぼ）平行なら交差しない
    constexpr float kEpsilon = 1e-6f;
    if (std::fabs(ray.direction.y) < kEpsilon) {
        return false;
    }

    float t = (planeY - ray.origin.y) / ray.direction.y;
    // 面が視線の後ろ側にある
    if (t < 0.0f) {
        return false;
    }

    *outPoint = ray.origin + ray.direction * t;
    return true;
}

Vector2 Camera::WorldToScreen(const Vector3& worldPos) const {
    Vector3 ndc = Transform(worldPos, viewProjection_);
    return {
        (ndc.x * 0.5f + 0.5f) * float(WinApp::kClientWidth),
        (1.0f - (ndc.y * 0.5f + 0.5f)) * float(WinApp::kClientHeight),
    };
}

} // namespace Engine
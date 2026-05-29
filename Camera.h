#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

class Camera {
public:
    Camera();
    ~Camera();

    void Update();

    const Matrix4x4& GetViewProjection() const { return viewProjection_; }
    const Matrix4x4& GetSpriteViewProjection() const { return spriteViewProjection_; }

    Vector3& GetTranslate() { return translate_; }
    Vector3& GetRotate() { return rotate_; }

private:
    Matrix4x4 viewProjection_{};
    Matrix4x4 spriteViewProjection_{};

    Vector3 rotate_ = { 0.0f, 0.0f, 0.0f };
    Vector3 translate_ = { 0.0f, 0.0f, -10.0f };

    float fovY_ = 0.45f;
    float nearZ_ = 0.1f;
    float farZ_ = 100.0f;
};
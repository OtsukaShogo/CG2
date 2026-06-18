#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

class Camera {
public:
	Camera();
	~Camera();

	void Initialize();
	void Update();

	const Matrix4x4& GetViewProjection() const { return viewProjection_; }
	const Matrix4x4& GetSpriteViewProjection() const { return spriteViewProjection_; }

	void AddRotation(float deltaX, float deltaY);

	Vector3& GetTranslate() { return translate_; }
	const Matrix4x4& GetRotateMatrix() const { return rotateMatrix_; }

private:
	Matrix4x4 viewMatrix_{};
	Matrix4x4 projMatrix_{};
	Matrix4x4 viewProjection_{};
	Matrix4x4 spriteViewProjection_{};
	Matrix4x4 rotateMatrix_{};

	Vector3 translate_ = { 0.0f, 0.0f, -10.0f };

	float fovY_ = 0.45f;
	float nearZ_ = 0.1f;
	float farZ_ = 100.0f;
};
#pragma once
#include "Camera.h"
#include "Matrix4x4.h"

class DebugCamera {
public:
	DebugCamera() = default;
	~DebugCamera() = default;

	void Initialize();
	void Update();

	const Matrix4x4& GetViewProjection() const { return camera_.GetViewProjection(); }

private:
	Camera camera_;

	static constexpr float kMoveSpeed   = 0.1f;
	static constexpr float kRotateSpeed = 0.005f;
	static constexpr float kZoomSpeed   = 0.01f;
};


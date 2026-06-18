#pragma once
#include <memory>
#include "Camera.h"
#include "DebugCamera.h"
#include "Matrix4x4.h"

class CameraManager {
public:
	static CameraManager* GetInstance();

	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	void Initialize();
	void Update();

	const Matrix4x4& GetViewProjection() const;
	const Matrix4x4& GetSpriteViewProjection() const;

	Camera* GetCamera() { return camera_.get(); }
	bool IsDebugCamera() const { return isDebugCamera_; }

#ifdef USE_IMGUI
	void DrawImGui();
#endif

private:
	CameraManager() = default;
	~CameraManager() = default;

	std::unique_ptr<Camera> camera_;
	std::unique_ptr<DebugCamera> debugCamera_;
	bool isDebugCamera_ = false;
};

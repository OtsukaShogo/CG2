#pragma once
#include <memory>
#include "Camera.h"
#include "DebugCamera.h"
#include "Matrix4x4.h"

namespace Engine {

/// <summary>
/// 通常カメラとデバッグカメラの切り替え・管理を行うクラス
/// </summary>
class CameraManager {
public:
	/// <summary>
	/// シングルトンインスタンスを取得する
	/// </summary>
	/// <returns>CameraManagerのインスタンス</returns>
	[[nodiscard]] static CameraManager* GetInstance();

	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	/// <summary>
	/// 通常カメラ・デバッグカメラを初期化する
	/// </summary>
	void Initialize();

	/// <summary>
	/// 入力に応じてデバッグカメラの切り替えと更新を行う
	/// </summary>
	void Update();

	/// <summary>
	/// 現在有効なカメラの3D用ビュープロジェクション行列を取得する
	/// </summary>
	/// <returns>3D用ビュープロジェクション行列</returns>
	const Matrix4x4& GetViewProjection() const;

	/// <summary>
	/// 現在有効なカメラのスプライト用ビュープロジェクション行列を取得する
	/// </summary>
	/// <returns>スプライト用ビュープロジェクション行列</returns>
	const Matrix4x4& GetSpriteViewProjection() const;

	/// <summary>
	/// 通常カメラを取得する
	/// </summary>
	/// <returns>通常カメラ</returns>
	Camera* GetCamera() { return camera_.get(); }

	/// <summary>
	/// デバッグカメラが有効かどうかを取得する
	/// </summary>
	/// <returns>デバッグカメラが有効な場合はtrue</returns>
	bool IsDebugCamera() const { return isDebugCamera_; }

#ifdef USE_IMGUI
	/// <summary>
	/// カメラ状態をImGuiに表示する
	/// </summary>
	void DrawImGui();
#endif

private:
	CameraManager() = default;
	~CameraManager() = default;

	std::unique_ptr<Camera> camera_;
	std::unique_ptr<DebugCamera> debugCamera_;
	bool isDebugCamera_ = false;
};

} // namespace Engine

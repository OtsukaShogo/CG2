#pragma once
#include "Camera.h"
#include "Matrix4x4.h"

namespace Engine {

/// <summary>
/// マウス・キーボード操作でカメラを自由に動かせるデバッグ用カメラクラス
/// </summary>
class DebugCamera {
public:
	DebugCamera() = default;
	~DebugCamera() = default;

	/// <summary>
	/// デバッグカメラを初期化する
	/// </summary>
	void Initialize();

	/// <summary>
	/// 入力に応じてカメラの移動・回転・ズームを更新する
	/// </summary>
	void Update();

	/// <summary>
	/// ビュープロジェクション行列を取得する
	/// </summary>
	/// <returns>ビュープロジェクション行列</returns>
	const Matrix4x4& GetViewProjection() const { return camera_.GetViewProjection(); }

private:
	Camera camera_;

	static constexpr float kMoveSpeed   = 0.1f;
	static constexpr float kRotateSpeed = 0.005f;
	static constexpr float kZoomSpeed   = 0.01f;
};

} // namespace Engine

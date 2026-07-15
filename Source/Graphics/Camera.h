#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"

namespace Engine {

/// <summary>
/// 3D/2D描画用のビュー・プロジェクション行列を管理するカメラクラス
/// </summary>
class Camera {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Camera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Camera();

	/// <summary>
	/// カメラの初期化を行う
	/// </summary>
	void Initialize();

	/// <summary>
	/// ビュー・プロジェクション行列を更新する
	/// </summary>
	void Update();

	/// <summary>
	/// 3Dオブジェクト用のビュープロジェクション行列を取得する
	/// </summary>
	/// <returns>3Dオブジェクト用ビュープロジェクション行列</returns>
	const Matrix4x4& GetViewProjection() const { return viewProjection_; }

	/// <summary>
	/// スプライト用のビュープロジェクション行列を取得する
	/// </summary>
	/// <returns>スプライト用ビュープロジェクション行列</returns>
	const Matrix4x4& GetSpriteViewProjection() const { return spriteViewProjection_; }

	/// <summary>
	/// カメラの回転量を加算する
	/// </summary>
	/// <param name="deltaX">X軸方向の回転増分</param>
	/// <param name="deltaY">Y軸方向の回転増分</param>
	void AddRotation(float deltaX, float deltaY);

	/// <summary>
	/// カメラの座標を取得する（変更可能）
	/// </summary>
	/// <returns>カメラ座標</returns>
	Vector3& GetTranslate() { return translate_; }

	/// <summary>
	/// カメラの回転行列を取得する
	/// </summary>
	/// <returns>カメラの回転行列</returns>
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

} // namespace Engine

#pragma once
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"
#include "../Math/Ray.h"
#include "../Math/Matrix4x4.h"

namespace Engine {

/// <summary>
/// 3D描画の投影方式
/// </summary>
enum class ProjectionType {
	kPerspective,  //!< 透視投影
	kOrthographic, //!< 平行投影
};

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
	/// カメラの回転をオイラー角で直接設定する（累積した回転は上書きされる）
	/// </summary>
	/// <param name="rotate">X・Y・Z軸の回転角（ラジアン）</param>
	void SetRotation(const Vector3& rotate);

	/// <summary>
	/// 垂直方向の視野角を取得する（変更可能）
	/// </summary>
	/// <returns>視野角（ラジアン）</returns>
	float& GetFovY() { return fovY_; }

	/// <summary>
	/// ニアクリップ距離を取得する（変更可能）
	/// </summary>
	/// <returns>ニアクリップ距離</returns>
	float& GetNearZ() { return nearZ_; }

	/// <summary>
	/// ファークリップ距離を取得する（変更可能）
	/// </summary>
	/// <returns>ファークリップ距離</returns>
	float& GetFarZ() { return farZ_; }

	/// <summary>
	/// 投影方式を設定する（反映は次の Update() 以降）
	/// </summary>
	/// <param name="type">投影方式</param>
	void SetProjectionType(ProjectionType type) { projectionType_ = type; }

	/// <summary>
	/// 現在の投影方式を取得する
	/// </summary>
	/// <returns>投影方式</returns>
	[[nodiscard]] ProjectionType GetProjectionType() const { return projectionType_; }

	/// <summary>
	/// 平行投影時の表示範囲の高さ（ワールド単位）を取得する（変更可能）。幅はアスペクト比から決まる
	/// </summary>
	/// <returns>平行投影の表示高さ</returns>
	float& GetOrthoHeight() { return orthoHeight_; }

	/// <summary>
	/// 画面上の点を通る視線（ワールド空間）を取得する。投影方式を問わず使える
	/// </summary>
	/// <param name="screenPos">スクリーン座標（左上原点、ピクセル単位）</param>
	/// <returns>画面上の点を通る視線</returns>
	[[nodiscard]] Ray ScreenToRay(const Vector2& screenPos) const;

	/// <summary>
	/// 画面上の点を通る視線と、水平面（y = planeY）との交点を求める。見下ろし型ゲームのマウスエイム用
	/// </summary>
	/// <param name="screenPos">スクリーン座標（左上原点、ピクセル単位）</param>
	/// <param name="planeY">交差させる水平面の高さ</param>
	/// <param name="outPoint">交点（交差した場合のみ書き込まれる）</param>
	/// <returns>交差した場合はtrue（視線が水平面と平行、または面が視線の後ろにある場合はfalse）</returns>
	[[nodiscard]] bool ScreenToWorldOnPlane(const Vector2& screenPos, float planeY, Vector3* outPoint) const;

	/// <summary>
	/// ワールド座標をスクリーン座標へ変換する
	/// </summary>
	/// <param name="worldPos">ワールド座標</param>
	/// <returns>スクリーン座標（左上原点、ピクセル単位）</returns>
	[[nodiscard]] Vector2 WorldToScreen(const Vector3& worldPos) const;

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
	Matrix4x4 inverseViewProjection_{}; // スクリーン→ワールド変換用
	Matrix4x4 spriteViewProjection_{};
	Matrix4x4 rotateMatrix_{};

	Vector3 translate_ = { 0.0f, 0.0f, -10.0f };

	float fovY_ = 0.45f;
	float nearZ_ = 0.1f;
	float farZ_ = 100.0f;

	ProjectionType projectionType_ = ProjectionType::kPerspective;
	float orthoHeight_ = 10.0f; // 平行投影時にカメラが映すワールド上の高さ
};

} // namespace Engine

#pragma once
#include "Camera.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

namespace Engine {

/// <summary>
/// 指定した対象を、一定の角度・距離から追従するカメラコントローラ。
/// 真上から見下ろす（pitch = 90度）ことも、斜め上から見る（pitch = 50〜70度程度）こともできる。
/// 注意: カメラの位置は Update() 内で決まる。追従対象の更新が終わった後に毎フレーム Update() を呼ぶこと
/// （Update() は内部で Camera::Update() も呼ぶので、直後の座標変換にも最新のカメラが使われる）
/// </summary>
class CameraController {
public:
	CameraController() = default;
	~CameraController() = default;

	/// <summary>
	/// 操作対象のカメラを設定する
	/// </summary>
	/// <param name="camera">操作するカメラ（所有はしない）</param>
	void Initialize(Camera* camera);

	/// <summary>
	/// カメラの位置・向きを更新する。有効（SetEnabled(true)）でないときは何もしない
	/// </summary>
	/// <param name="deltaTime">経過時間（秒）</param>
	void Update(float deltaTime);

	/// <summary>
	/// 追従のなめらかさを無視して、カメラを即座に目標位置へ移動する（シーン開始時やワープ時用）
	/// </summary>
	void Snap();

	/// <summary>
	/// 追従対象の座標を設定する。ポインタ版と異なり、呼び出した時点の値で固定される
	/// </summary>
	/// <param name="position">追従する座標</param>
	void SetTargetPosition(const Vector3& position);

	/// <summary>
	/// 追従対象の座標へのポインタを設定する。毎フレーム最新の値を参照する（nullptrで解除）。
	/// 指す先が破棄される前に必ず nullptr を設定すること
	/// </summary>
	/// <param name="target">追従する座標へのポインタ</param>
	void SetTarget(const Vector3* target) { target_ = target; }

	/// <summary>
	/// コントローラの有効・無効を設定する。無効の間はカメラに触れない
	/// </summary>
	/// <param name="enabled">有効にする場合はtrue</param>
	void SetEnabled(bool enabled) { enabled_ = enabled; }

	/// <summary>
	/// 有効かどうかを取得する
	/// </summary>
	/// <returns>有効な場合はtrue</returns>
	[[nodiscard]] bool IsEnabled() const { return enabled_; }

	/// <summary>
	/// 追従範囲（注視点のXZ範囲）を設定する。ステージの外が映りすぎないようにするために使う
	/// 画面の端ではなく注視点（画面中心）を制限する
	/// </summary>
	/// <param name="min">注視点のXZ最小値</param>
	/// <param name="max">注視点のXZ最大値</param>
	void SetBounds(const Vector2& min, const Vector2& max);

	/// <summary>
	/// 追従範囲の制限を解除する
	/// </summary>
	void ClearBounds() { useBounds_ = false; }

	/// <summary>
	/// 見下ろし角（ラジアン）を取得する（変更可能）。π/2で真上、小さくするほど斜めになる
	/// </summary>
	/// <returns>見下ろし角</returns>
	float& GetPitch() { return pitch_; }

	/// <summary>
	/// Y軸まわりの回転角（ラジアン）を取得する（変更可能）。0のとき画面の上がワールド+Z方向
	/// </summary>
	/// <returns>Y軸回転角</returns>
	float& GetYaw() { return yaw_; }

	/// <summary>
	/// 注視点からカメラまでの距離を取得する（変更可能）。クリップ範囲に収まる値にする
	/// </summary>
	/// <returns>カメラ距離</returns>
	float& GetDistance() { return distance_; }

	/// <summary>
	/// 追従対象から注視点へのオフセットを取得する（変更可能）。プレイヤーを画面中心からずらしたいときに使う
	/// </summary>
	/// <returns>注視点オフセット</returns>
	Vector3& GetOffset() { return offset_; }

	/// <summary>
	/// 追従の速さ（1/秒）を取得する（変更可能）。大きいほど素早く追いつき、0以下で遅れなしの即時追従
	/// </summary>
	/// <returns>追従の速さ</returns>
	float& GetFollowSpeed() { return followSpeed_; }

	/// <summary>
	/// 現在の注視点を取得する
	/// </summary>
	/// <returns>注視点</returns>
	[[nodiscard]] const Vector3& GetFocusPoint() const { return focusPoint_; }

#ifdef USE_IMGUI
	/// <summary>
	/// コントローラの設定をImGuiで編集できるように表示する
	/// </summary>
	void DrawImGui();
#endif

private:
	/// <summary>
	/// 追従対象とオフセットから、注視点の目標位置を求める（追従範囲の制限も適用）
	/// </summary>
	[[nodiscard]] Vector3 CalcDesiredFocus() const;

	/// <summary>
	/// 現在の注視点・角度・距離からカメラの回転と位置を反映する
	/// </summary>
	void ApplyToCamera();

private:
	Camera* camera_ = nullptr;
	const Vector3* target_ = nullptr;
	Vector3 targetValue_ = { 0.0f, 0.0f, 0.0f }; // SetTargetPosition() で指定された固定の座標

	bool enabled_ = false;

	float pitch_ = 1.5707963f; // 既定は真上から見下ろす
	float yaw_ = 0.0f;
	float distance_ = 10.0f;
	Vector3 offset_ = { 0.0f, 0.0f, 0.0f };
	float followSpeed_ = 8.0f;

	bool useBounds_ = false;
	Vector2 boundsMin_ = { 0.0f, 0.0f };
	Vector2 boundsMax_ = { 0.0f, 0.0f };

	Vector3 focusPoint_ = { 0.0f, 0.0f, 0.0f };
};

} // namespace Engine

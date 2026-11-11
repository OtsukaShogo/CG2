#pragma once
#include "../Math/AABB.h"
#include "../Math/Vector3.h"

namespace Engine {

/// <summary>
/// 範囲内にいる物体へ加速度を与える領域
/// </summary>
struct AccelerationField {
	Vector3 acceleration; //!< 加速度
	AABB area;            //!< 範囲
};

/// <summary>
/// 風を表すクラス。パーティクルやゲームオブジェクトが共通で参照できる
/// </summary>
class Wind {
public:
	/// <summary>
	/// 風の加速度を設定する
	/// </summary>
	/// <param name="acceleration">加速度</param>
	void SetAcceleration(const Vector3& acceleration) { field_.acceleration = acceleration; }

	/// <summary>
	/// 風が吹く範囲を設定する
	/// </summary>
	/// <param name="area">範囲</param>
	void SetArea(const AABB& area) { field_.area = area; }

	/// <summary>
	/// 風の有効/無効を設定する
	/// </summary>
	/// <param name="enabled">有効ならtrue</param>
	void SetEnabled(bool enabled) { enabled_ = enabled; }

	/// <summary>
	/// 風の領域を取得する（変更可能。ImGuiなどから直接編集する用）
	/// </summary>
	/// <returns>風の領域</returns>
	AccelerationField& GetField() { return field_; }

	/// <summary>
	/// 有効フラグを取得する（変更可能）
	/// </summary>
	/// <returns>有効フラグ</returns>
	bool& GetEnabled() { return enabled_; }

	/// <summary>
	/// 指定した位置に風が当たるかを判定する
	/// </summary>
	/// <param name="position">判定する位置</param>
	/// <returns>有効かつ範囲内ならtrue</returns>
	[[nodiscard]] bool IsBlowing(const Vector3& position) const {
		return enabled_ && IsInside(field_.area, position);
	}

	/// <summary>
	/// 指定した位置での加速度を取得する
	/// </summary>
	/// <param name="position">判定する位置</param>
	/// <returns>風が当たるなら加速度、当たらなければゼロベクトル</returns>
	[[nodiscard]] Vector3 GetAccelerationAt(const Vector3& position) const {
		return IsBlowing(position) ? field_.acceleration : Vector3{ 0.0f, 0.0f, 0.0f };
	}

private:
	AccelerationField field_{ { 0.0f, 0.0f, 0.0f }, { { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } } };
	bool enabled_ = true;
};

} // namespace Engine

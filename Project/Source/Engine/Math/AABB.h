#pragma once
#include "Vector3.h"

namespace Engine {

/// <summary>
/// 軸平行境界ボックス（Axis-Aligned Bounding Box）
/// </summary>
struct AABB {
	Vector3 min; //!< 最小点
	Vector3 max; //!< 最大点
};

/// <summary>
/// 点がAABBの内側（境界を含む）にあるかを判定する
/// </summary>
/// <param name="aabb">判定対象のAABB</param>
/// <param name="point">判定する点</param>
/// <returns>内側にあればtrue</returns>
[[nodiscard]] inline bool IsInside(const AABB& aabb, const Vector3& point) {
	return (point.x >= aabb.min.x && point.x <= aabb.max.x) &&
		(point.y >= aabb.min.y && point.y <= aabb.max.y) &&
		(point.z >= aabb.min.z && point.z <= aabb.max.z);
}

} // namespace Engine

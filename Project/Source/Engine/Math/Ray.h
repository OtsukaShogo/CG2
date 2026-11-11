#pragma once
#include "Vector3.h"

namespace Engine {

/// <summary>
/// 半直線（始点と正規化された方向）
/// </summary>
struct Ray {
	Vector3 origin;    //!< 始点
	Vector3 direction; //!< 方向（正規化済み）
};

} // namespace Engine

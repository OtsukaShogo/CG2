#pragma once
#include"Vector4.h"
#include"Vector2.h"
#include"Vector3.h"

namespace Engine {

/// <summary>
/// 頂点1つ分のデータ（座標・UV座標・法線）
/// </summary>
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

} // namespace Engine

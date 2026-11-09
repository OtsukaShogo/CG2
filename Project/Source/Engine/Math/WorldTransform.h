#pragma once
#include"Vector3.h"

namespace Engine {

/// <summary>
/// オブジェクトのワールド空間上の拡大縮小・回転・平行移動を表す
/// </summary>
struct WorldTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

} // namespace Engine

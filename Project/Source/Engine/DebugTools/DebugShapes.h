#pragma once
#include <cstdint>
#include <vector>
#include "../Math/Matrix4x4.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

namespace Engine {

/// <summary>
/// 線分1本（始点と終点）
/// </summary>
struct DebugLine {
	Vector3 start;
	Vector3 end;
};

using DebugLineList = std::vector<DebugLine>;

/// <summary>
/// デバッグ表示用の図形を、線分の集まりとして生成する関数群。
/// D3D12に依存しない（CPUだけで動く）ので、単体テストできる。
/// すべて、結果を out の末尾へ追加する。
/// </summary>
namespace DebugShapes {

/// <summary>
/// 円（正多角形）。center を中心に、u・v が張る平面上に描く
/// </summary>
/// <param name="u">平面上の軸（長さ1・vと直交）</param>
/// <param name="v">平面上のもう1つの軸（長さ1・uと直交）</param>
/// <param name="segments">分割数（3未満は3にする）</param>
void AppendCircle(DebugLineList& out, const Vector3& center, const Vector3& u, const Vector3& v, float radius, uint32_t segments);

/// <summary>
/// 円。normal を法線とする平面上に描く（normal は正規化されていなくてよい。ゼロベクトルなら何も追加しない）
/// </summary>
void AppendCircle(DebugLineList& out, const Vector3& center, const Vector3& normal, float radius, uint32_t segments);

/// <summary>
/// 球（XY・YZ・XZ平面の3つの円で表す）
/// </summary>
void AppendSphere(DebugLineList& out, const Vector3& center, float radius, uint32_t segments);

/// <summary>
/// 軸に平行な直方体（AABB）の12本の辺
/// </summary>
void AppendBox(DebugLineList& out, const Vector3& min, const Vector3& max);

/// <summary>
/// ワールド行列で変換した直方体の12本の辺。大きさ1・原点中心の立方体（PrimitiveType::kBoxと同じ）が元になる。
/// 回転した箱（OBB）は、MakeAffineMatrix でスケール・回転・移動を指定して渡す
/// </summary>
void AppendBox(DebugLineList& out, const Matrix4x4& world);

/// <summary>
/// 水平な長方形（XZ平面）の4本の辺
/// </summary>
/// <param name="center">中心</param>
/// <param name="halfSize">X方向・Z方向の半分の長さ</param>
void AppendRect(DebugLineList& out, const Vector3& center, const Vector2& halfSize);

/// <summary>
/// 水平な格子（XZ平面）。center を中心に、一辺 halfExtent*2 の範囲へ、step 間隔で線を引く
/// </summary>
/// <param name="halfExtent">中心から端までの長さ</param>
/// <param name="step">線の間隔（0以下なら何も追加しない）</param>
void AppendGrid(DebugLineList& out, const Vector3& center, float halfExtent, float step);

/// <summary>
/// 矢印（始点から終点への線と、先端の矢じり4本）
/// </summary>
/// <param name="headLength">矢じりの長さ（矢印全体の長さの半分まで）</param>
void AppendArrow(DebugLineList& out, const Vector3& from, const Vector3& to, float headLength);

} // namespace DebugShapes

} // namespace Engine

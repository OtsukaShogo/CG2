#pragma once
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

namespace Engine {

// 見下ろし型ゲーム用の、2D（水平面）の当たり判定。
// 水平面は XZ 平面で、Vector2 の x がワールドの X、y がワールドの Z に対応する（ToXZ / FromXZ で変換する）。
// 「接している」（距離がちょうど半径の和など）も、当たっている扱いにする。
//
// 注意: Vector2 などは集成体なので、{x, y} を直接引数に渡すと、オーバーロードが曖昧になることがある。
//       形の型（Circle2D など）を明示して渡すこと。

/// <summary>
/// 円
/// </summary>
struct Circle2D {
	Vector2 center = { 0.0f, 0.0f };
	float radius = 0.0f;
};

/// <summary>
/// 軸に平行な矩形（AABB）
/// </summary>
struct AABB2D {
	Vector2 min = { 0.0f, 0.0f }; //!< 最小点（x・yとも小さいほう）
	Vector2 max = { 0.0f, 0.0f }; //!< 最大点
};

/// <summary>
/// 線分（弾の1フレームの移動軌跡など）
/// </summary>
struct Segment2D {
	Vector2 start = { 0.0f, 0.0f };
	Vector2 end = { 0.0f, 0.0f };
};

/// <summary>
/// 衝突の詳細（めり込みの解消に使う）。
/// normal は「第2引数の形から、第1引数の形へ向かう」向き（長さ1）。
/// 第1引数の形を normal * depth だけ動かすと、ちょうど接する状態に戻る
/// </summary>
struct Contact2D {
	bool hit = false;
	Vector2 normal = { 0.0f, 0.0f };     //!< 第1引数を押し戻す向き（長さ1。hit=falseのときは未定義）
	float depth = 0.0f; //!< めり込んだ深さ（0以上）
};

/// <summary>
/// 線分やレイキャストの当たり結果
/// </summary>
struct RayHit2D {
	bool hit = false;
	float t = 0.0f;      //!< 線分上の位置（0=始点、1=終点）。最初に当たる点
	Vector2 point = { 0.0f, 0.0f };       //!< 最初に当たる点
	Vector2 normal = { 0.0f, 0.0f };      //!< 当たった面の外向きの法線（始点がすでに内側のときは {0,0}）
};

/// <summary>
/// 3Dの座標を水平面（XZ）の2D座標にする
/// </summary>
[[nodiscard]] inline Vector2 ToXZ(const Vector3& v) { return Vector2{ v.x, v.z }; }

/// <summary>
/// 水平面の2D座標を、高さ y の3D座標にする
/// </summary>
[[nodiscard]] inline Vector3 FromXZ(const Vector2& v, float y = 0.0f) { return Vector3{ v.x, y, v.y }; }

// ---- 最近接点 ----

/// <summary>
/// 矩形上（内側を含む）で、点に最も近い点を求める
/// </summary>
[[nodiscard]] Vector2 ClosestPoint(const Vector2& point, const AABB2D& aabb);

/// <summary>
/// 線分上で、点に最も近い点を求める（長さ0の線分は始点を返す）
/// </summary>
[[nodiscard]] Vector2 ClosestPoint(const Vector2& point, const Segment2D& segment);

// ---- 判定のみ（当たっているか） ----

[[nodiscard]] bool Intersects(const Circle2D& a, const Circle2D& b);
[[nodiscard]] bool Intersects(const Circle2D& circle, const AABB2D& aabb);
[[nodiscard]] bool Intersects(const AABB2D& aabb, const Circle2D& circle);
[[nodiscard]] bool Intersects(const AABB2D& a, const AABB2D& b);
[[nodiscard]] bool Intersects(const Segment2D& segment, const Circle2D& circle);
[[nodiscard]] bool Intersects(const Circle2D& circle, const Segment2D& segment);
[[nodiscard]] bool Intersects(const Segment2D& segment, const AABB2D& aabb);
[[nodiscard]] bool Intersects(const AABB2D& aabb, const Segment2D& segment);
[[nodiscard]] bool Intersects(const Segment2D& a, const Segment2D& b);

/// <summary>
/// 点が円・矩形の内側（境界を含む）にあるか
/// </summary>
[[nodiscard]] bool Contains(const Circle2D& circle, const Vector2& point);
[[nodiscard]] bool Contains(const AABB2D& aabb, const Vector2& point);

// ---- 衝突の詳細（押し戻しに使う） ----

/// <summary>
/// 円どうしの衝突。同じ位置（中心が一致）の場合、normalは {1,0} になる
/// </summary>
[[nodiscard]] Contact2D Collide(const Circle2D& a, const Circle2D& b);

/// <summary>
/// 円と矩形の衝突。円の中心が矩形の内側にある場合は、最も近い面から押し出す向きになる
/// </summary>
[[nodiscard]] Contact2D Collide(const Circle2D& circle, const AABB2D& aabb);

/// <summary>
/// 矩形と円の衝突（normal は矩形を押し戻す向き）
/// </summary>
[[nodiscard]] Contact2D Collide(const AABB2D& aabb, const Circle2D& circle);

/// <summary>
/// 矩形どうしの衝突。重なりが最も浅い軸の向きに押し戻す
/// </summary>
[[nodiscard]] Contact2D Collide(const AABB2D& a, const AABB2D& b);

// ---- 線分のキャスト（最初に当たる位置） ----

/// <summary>
/// 線分を始点から終点へ進めたとき、円に最初に当たる位置を求める。
/// 速い弾が、1フレームで敵をすり抜けてしまうのを防ぐ（弾の前フレームの位置→今の位置を線分にする）。
/// 始点がすでに円の内側なら、t=0 で当たる
/// </summary>
[[nodiscard]] RayHit2D Raycast(const Segment2D& segment, const Circle2D& circle);

/// <summary>
/// 線分を始点から終点へ進めたとき、矩形に最初に当たる位置と面の法線を求める。
/// 始点がすでに矩形の内側なら、t=0・normal={0,0} で当たる
/// </summary>
[[nodiscard]] RayHit2D Raycast(const Segment2D& segment, const AABB2D& aabb);

} // namespace Engine

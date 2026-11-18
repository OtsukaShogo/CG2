#pragma once
#include <limits>
#include "../Math/AABB.h"
#include "../Math/Quaternion.h"
#include "../Math/Ray.h"
#include "../Math/Vector3.h"

namespace Engine {

// 3Dの当たり判定。
//   形: Sphere（球）、AABB（軸に平行な箱。Math/AABB.h）、OBB（回転した箱）、Capsule（カプセル）、Plane（平面）、
//       Segment3D（線分）、Ray（半直線。Math/Ray.h）
//   判定のみ: Intersects(a, b)  …… 当たっているか
//   押し戻し: Collide(a, b)     …… 当たっていれば、normal（aをbから離す向き）と depth（めり込み量）を返す
//   レイキャスト: Raycast(線分またはレイ, 形) …… 最初に当たる位置・法線を返す
// 「接している」も、当たっている扱いにする。引数の順は、どの組み合わせでも入れ替えて呼べる
//（押し戻しの normal は「第1引数を動かす向き」なので、入れ替えると逆向きになる）。
//
// 注意: Vector3 などは集成体なので、{x, y, z} を直接引数に渡すと、オーバーロードが曖昧になることがある。
//       形の型（Sphere など）を明示して渡すこと。

/// <summary>
/// 球
/// </summary>
struct Sphere {
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	float radius = 0.0f;
};

/// <summary>
/// 線分
/// </summary>
struct Segment3D {
	Vector3 start = { 0.0f, 0.0f, 0.0f };
	Vector3 end = { 0.0f, 0.0f, 0.0f };
};

/// <summary>
/// カプセル（線分から radius 以内の範囲。立っているキャラクターや、細長い弾に向く）
/// </summary>
struct Capsule {
	Segment3D segment;
	float radius = 0.0f;
};

/// <summary>
/// 回転した箱（OBB）。axes は互いに直交する長さ1の3軸（箱の向き）で、halfExtents は各軸方向の半分の長さ
/// </summary>
struct OBB {
	Vector3 center = { 0.0f, 0.0f, 0.0f };
	Vector3 axes[3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	Vector3 halfExtents = { 0.5f, 0.5f, 0.5f };
};

/// <summary>
/// 平面。normal（長さ1）と、原点からの距離 distance で、dot(normal, p) == distance を満たす点 p の集まり。
/// normal が向いている側が「表」
/// </summary>
struct Plane {
	Vector3 normal = { 0.0f, 1.0f, 0.0f };
	float distance = 0.0f;
};

/// <summary>
/// 衝突の詳細（めり込みの解消に使う）。
/// normal は「第2引数の形から、第1引数の形へ向かう」向き（長さ1）。
/// 第1引数の形を normal * depth だけ動かすと、ちょうど接する状態に戻る
/// </summary>
struct Contact3D {
	bool hit = false;
	Vector3 normal = { 0.0f, 0.0f, 0.0f }; //!< 第1引数を押し戻す向き（長さ1。hit=falseのときは未定義）
	float depth = 0.0f;                    //!< めり込んだ深さ（0以上）
};

/// <summary>
/// レイキャストの結果
/// </summary>
struct RayHit3D {
	bool hit = false;
	float t = 0.0f;                        //!< 始点からの距離（線分の場合は 0=始点、1=終点の割合）。最初に当たる点
	Vector3 point = { 0.0f, 0.0f, 0.0f };  //!< 最初に当たる点
	Vector3 normal = { 0.0f, 0.0f, 0.0f }; //!< 当たった面の外向きの法線（始点がすでに内側のときは {0,0,0}）
};

// ---- 形の作成 ----

/// <summary>
/// 中心・各軸方向の半分の長さ・向き（クォータニオン）から、OBBを作る
/// </summary>
[[nodiscard]] OBB MakeOBB(const Vector3& center, const Vector3& halfExtents, const Quaternion& rotation);

/// <summary>
/// 中心・各軸方向の半分の長さ・向き（オイラー角。MakeAffineMatrix と同じ X→Y→Z の順）から、OBBを作る。
/// Modelの座標・スケール・回転から、見た目と同じ向きの判定を作るのに使う
/// </summary>
[[nodiscard]] OBB MakeOBB(const Vector3& center, const Vector3& halfExtents, const Vector3& eulerRadians);

/// <summary>
/// AABB を OBB にする（向きは軸に平行）
/// </summary>
[[nodiscard]] OBB ToOBB(const AABB& aabb);

/// <summary>
/// 地面に立っているキャラクター用のカプセルを作る（Y軸方向）
/// </summary>
/// <param name="foot">足元の位置</param>
/// <param name="height">全体の高さ（半径の2倍より小さい場合は、球になる）</param>
/// <param name="radius">半径</param>
[[nodiscard]] Capsule MakeUprightCapsule(const Vector3& foot, float height, float radius);

/// <summary>
/// 平面上の1点と法線（正規化されていなくてよい）から、平面を作る
/// </summary>
[[nodiscard]] Plane MakePlane(const Vector3& point, const Vector3& normal);

/// <summary>
/// OBBの8個の頂点を求める（ビット0=軸0・ビット1=軸1・ビット2=軸2。1なら正の側）
/// </summary>
void GetCorners(const OBB& obb, Vector3 outCorners[8]);

// ---- 最近接点・距離 ----

/// <summary>
/// 箱（内側を含む）の上で、点に最も近い点を求める
/// </summary>
[[nodiscard]] Vector3 ClosestPoint(const Vector3& point, const AABB& aabb);
[[nodiscard]] Vector3 ClosestPoint(const Vector3& point, const OBB& obb);

/// <summary>
/// 線分上で、点に最も近い点を求める（長さ0の線分は始点を返す）
/// </summary>
[[nodiscard]] Vector3 ClosestPoint(const Vector3& point, const Segment3D& segment);

/// <summary>
/// 点から平面までの符号付き距離（表側が正）
/// </summary>
[[nodiscard]] float SignedDistance(const Plane& plane, const Vector3& point);

/// <summary>
/// 2つの線分の最近接点を求め、その距離の2乗を返す（平行・長さ0の線分にも対応）
/// </summary>
/// <param name="outA">a上の最近接点</param>
/// <param name="outB">b上の最近接点</param>
[[nodiscard]] float ClosestPoints(const Segment3D& a, const Segment3D& b, Vector3* outA, Vector3* outB);

/// <summary>
/// 線分と箱の最近接点を求め、その距離の2乗を返す。交差している場合は 0（outSegment と outBox は同じ点）
/// </summary>
/// <param name="outSegment">線分上の最近接点</param>
/// <param name="outBox">箱上の最近接点</param>
[[nodiscard]] float ClosestPoints(const Segment3D& segment, const OBB& obb, Vector3* outSegment, Vector3* outBox);
[[nodiscard]] float ClosestPoints(const Segment3D& segment, const AABB& aabb, Vector3* outSegment, Vector3* outBox);

/// <summary>
/// 点が形の内側（境界を含む）にあるか
/// </summary>
[[nodiscard]] bool Contains(const Sphere& sphere, const Vector3& point);
[[nodiscard]] bool Contains(const OBB& obb, const Vector3& point);
[[nodiscard]] bool Contains(const Capsule& capsule, const Vector3& point);

// ---- 判定のみ（当たっているか） ----

[[nodiscard]] bool Intersects(const Sphere& a, const Sphere& b);
[[nodiscard]] bool Intersects(const Sphere& sphere, const AABB& aabb);
[[nodiscard]] bool Intersects(const Sphere& sphere, const OBB& obb);
[[nodiscard]] bool Intersects(const Sphere& sphere, const Capsule& capsule);
[[nodiscard]] bool Intersects(const Sphere& sphere, const Plane& plane);
[[nodiscard]] bool Intersects(const AABB& a, const AABB& b);
[[nodiscard]] bool Intersects(const AABB& aabb, const OBB& obb);
[[nodiscard]] bool Intersects(const OBB& a, const OBB& b);
[[nodiscard]] bool Intersects(const Capsule& a, const Capsule& b);
[[nodiscard]] bool Intersects(const Capsule& capsule, const AABB& aabb);
[[nodiscard]] bool Intersects(const Capsule& capsule, const OBB& obb);
[[nodiscard]] bool Intersects(const Capsule& capsule, const Plane& plane);
[[nodiscard]] bool Intersects(const AABB& aabb, const Plane& plane);
[[nodiscard]] bool Intersects(const OBB& obb, const Plane& plane);
[[nodiscard]] bool Intersects(const Segment3D& segment, const Sphere& sphere);
[[nodiscard]] bool Intersects(const Segment3D& segment, const AABB& aabb);
[[nodiscard]] bool Intersects(const Segment3D& segment, const OBB& obb);
[[nodiscard]] bool Intersects(const Segment3D& segment, const Capsule& capsule);
[[nodiscard]] bool Intersects(const Segment3D& segment, const Plane& plane);

// 引数の順を入れ替えた形
[[nodiscard]] inline bool Intersects(const AABB& aabb, const Sphere& sphere) { return Intersects(sphere, aabb); }
[[nodiscard]] inline bool Intersects(const OBB& obb, const Sphere& sphere) { return Intersects(sphere, obb); }
[[nodiscard]] inline bool Intersects(const Capsule& capsule, const Sphere& sphere) { return Intersects(sphere, capsule); }
[[nodiscard]] inline bool Intersects(const Plane& plane, const Sphere& sphere) { return Intersects(sphere, plane); }
[[nodiscard]] inline bool Intersects(const OBB& obb, const AABB& aabb) { return Intersects(aabb, obb); }
[[nodiscard]] inline bool Intersects(const AABB& aabb, const Capsule& capsule) { return Intersects(capsule, aabb); }
[[nodiscard]] inline bool Intersects(const OBB& obb, const Capsule& capsule) { return Intersects(capsule, obb); }
[[nodiscard]] inline bool Intersects(const Plane& plane, const Capsule& capsule) { return Intersects(capsule, plane); }
[[nodiscard]] inline bool Intersects(const Plane& plane, const AABB& aabb) { return Intersects(aabb, plane); }
[[nodiscard]] inline bool Intersects(const Plane& plane, const OBB& obb) { return Intersects(obb, plane); }
[[nodiscard]] inline bool Intersects(const Sphere& sphere, const Segment3D& segment) { return Intersects(segment, sphere); }
[[nodiscard]] inline bool Intersects(const AABB& aabb, const Segment3D& segment) { return Intersects(segment, aabb); }
[[nodiscard]] inline bool Intersects(const OBB& obb, const Segment3D& segment) { return Intersects(segment, obb); }
[[nodiscard]] inline bool Intersects(const Capsule& capsule, const Segment3D& segment) { return Intersects(segment, capsule); }
[[nodiscard]] inline bool Intersects(const Plane& plane, const Segment3D& segment) { return Intersects(segment, plane); }

// ---- 衝突の詳細（押し戻しに使う） ----

/// <summary>
/// 球どうし。中心が一致して向きが決まらない場合、normal は {0,1,0} になる
/// </summary>
[[nodiscard]] Contact3D Collide(const Sphere& a, const Sphere& b);
[[nodiscard]] Contact3D Collide(const Sphere& sphere, const AABB& aabb);
[[nodiscard]] Contact3D Collide(const Sphere& sphere, const OBB& obb);
[[nodiscard]] Contact3D Collide(const Sphere& sphere, const Capsule& capsule);
[[nodiscard]] Contact3D Collide(const Sphere& sphere, const Plane& plane);

/// <summary>
/// 箱どうし。重なりが最も浅い分離軸の向きに押し戻す
/// </summary>
[[nodiscard]] Contact3D Collide(const AABB& a, const AABB& b);
[[nodiscard]] Contact3D Collide(const AABB& aabb, const OBB& obb);
[[nodiscard]] Contact3D Collide(const OBB& a, const OBB& b);

/// <summary>
/// カプセルどうし。軸が交差している場合は、2本の軸に垂直な向きに押し戻す
/// </summary>
[[nodiscard]] Contact3D Collide(const Capsule& a, const Capsule& b);

/// <summary>
/// カプセルと箱。軸が箱の内側に入り込んでいる場合は、最も近い面から押し出す
/// </summary>
[[nodiscard]] Contact3D Collide(const Capsule& capsule, const AABB& aabb);
[[nodiscard]] Contact3D Collide(const Capsule& capsule, const OBB& obb);

/// <summary>
/// カプセルと平面。軸が平面を跨いでいる場合は、軸の中点がある側へ押し出す（足元の地面との押し戻しなど）
/// </summary>
[[nodiscard]] Contact3D Collide(const Capsule& capsule, const Plane& plane);

// 引数の順を入れ替えた形（normal は、新しい第1引数を押し戻す向き）
[[nodiscard]] Contact3D FlipContact(Contact3D contact);
[[nodiscard]] inline Contact3D Collide(const AABB& aabb, const Sphere& sphere) { return FlipContact(Collide(sphere, aabb)); }
[[nodiscard]] inline Contact3D Collide(const OBB& obb, const Sphere& sphere) { return FlipContact(Collide(sphere, obb)); }
[[nodiscard]] inline Contact3D Collide(const Capsule& capsule, const Sphere& sphere) { return FlipContact(Collide(sphere, capsule)); }
[[nodiscard]] inline Contact3D Collide(const Plane& plane, const Sphere& sphere) { return FlipContact(Collide(sphere, plane)); }
[[nodiscard]] inline Contact3D Collide(const OBB& obb, const AABB& aabb) { return FlipContact(Collide(aabb, obb)); }
[[nodiscard]] inline Contact3D Collide(const AABB& aabb, const Capsule& capsule) { return FlipContact(Collide(capsule, aabb)); }
[[nodiscard]] inline Contact3D Collide(const OBB& obb, const Capsule& capsule) { return FlipContact(Collide(capsule, obb)); }
[[nodiscard]] inline Contact3D Collide(const Plane& plane, const Capsule& capsule) { return FlipContact(Collide(capsule, plane)); }

// ---- レイキャスト（最初に当たる位置） ----

/// <summary>
/// 線分を始点から終点へ進めたとき、形に最初に当たる位置を求める（t は 0〜1 の割合）。
/// 速い弾が、1フレームで敵をすり抜けてしまうのを防ぐ（弾の前フレームの位置→今の位置を線分にする）。
/// 始点がすでに形の内側なら、t=0・normal={0,0,0} で当たる（平面は、始点の側を表として扱う）
/// </summary>
[[nodiscard]] RayHit3D Raycast(const Segment3D& segment, const Sphere& sphere);
[[nodiscard]] RayHit3D Raycast(const Segment3D& segment, const AABB& aabb);
[[nodiscard]] RayHit3D Raycast(const Segment3D& segment, const OBB& obb);
[[nodiscard]] RayHit3D Raycast(const Segment3D& segment, const Capsule& capsule);
[[nodiscard]] RayHit3D Raycast(const Segment3D& segment, const Plane& plane);

/// <summary>
/// レイ（半直線）を進めたとき、形に最初に当たる位置を求める（t は始点からの距離）。
/// マウス照準（Camera::ScreenToRay で作ったレイ）で、敵や地面を選ぶのに使う
/// </summary>
/// <param name="maxDistance">調べる最大距離（既定は無限）</param>
[[nodiscard]] RayHit3D Raycast(const Ray& ray, const Sphere& sphere, float maxDistance = (std::numeric_limits<float>::infinity)());
[[nodiscard]] RayHit3D Raycast(const Ray& ray, const AABB& aabb, float maxDistance = (std::numeric_limits<float>::infinity)());
[[nodiscard]] RayHit3D Raycast(const Ray& ray, const OBB& obb, float maxDistance = (std::numeric_limits<float>::infinity)());
[[nodiscard]] RayHit3D Raycast(const Ray& ray, const Capsule& capsule, float maxDistance = (std::numeric_limits<float>::infinity)());
[[nodiscard]] RayHit3D Raycast(const Ray& ray, const Plane& plane, float maxDistance = (std::numeric_limits<float>::infinity)());

} // namespace Engine

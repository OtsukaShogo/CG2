#include "TestFramework.h"
#include "Source/Engine/Collision/Collision2D.h"

#include <cmath>

using namespace Engine;

namespace {

Circle2D C(float x, float y, float r) { return Circle2D{ Vector2{ x, y }, r }; }
AABB2D R(float minX, float minY, float maxX, float maxY) { return AABB2D{ Vector2{ minX, minY }, Vector2{ maxX, maxY } }; }
Segment2D S(float x0, float y0, float x1, float y1) { return Segment2D{ Vector2{ x0, y0 }, Vector2{ x1, y1 } }; }
Vector2 V(float x, float y) { return Vector2{ x, y }; }

bool Near(const Vector2& a, const Vector2& b, float eps = 1e-5f) {
	return std::abs(a.x - b.x) <= eps && std::abs(a.y - b.y) <= eps;
}

float Len(const Vector2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

} // namespace

TEST_CASE(Collision_XZConversion) {
	CHECK(Near(ToXZ(Vector3{ 1.0f, 2.0f, 3.0f }), V(1, 3)));
	Vector3 v = FromXZ(V(4, 5), 6.0f);
	CHECK(v.x == 4.0f);
	CHECK(v.y == 6.0f);
	CHECK(v.z == 5.0f);
}

TEST_CASE(Collision_ClosestPoint) {
	// 矩形
	CHECK(Near(ClosestPoint(V(5, 5), R(0, 0, 2, 2)), V(2, 2)));   // 角
	CHECK(Near(ClosestPoint(V(1, 5), R(0, 0, 2, 2)), V(1, 2)));   // 辺
	CHECK(Near(ClosestPoint(V(1, 1), R(0, 0, 2, 2)), V(1, 1)));   // 内側はそのまま
	// 線分
	CHECK(Near(ClosestPoint(V(5, 3), S(0, 0, 10, 0)), V(5, 0)));  // 途中
	CHECK(Near(ClosestPoint(V(-5, 3), S(0, 0, 10, 0)), V(0, 0))); // 始点側の外
	CHECK(Near(ClosestPoint(V(15, 3), S(0, 0, 10, 0)), V(10, 0)));// 終点側の外
	// 長さ0の線分でも、NaNにならず始点を返す
	Vector2 p = ClosestPoint(V(3, 4), S(1, 1, 1, 1));
	CHECK(Near(p, V(1, 1)));
}

TEST_CASE(Collision_Contains) {
	CHECK(Contains(C(0, 0, 1), V(1, 0)));    // 円周上は内側
	CHECK(!Contains(C(0, 0, 1), V(1.01f, 0)));
	CHECK(Contains(R(0, 0, 2, 2), V(2, 2))); // 角は内側
	CHECK(!Contains(R(0, 0, 2, 2), V(2.01f, 1)));
}

TEST_CASE(Collision_CircleCircle) {
	CHECK(Intersects(C(0, 0, 1), C(1.5f, 0, 1)));   // 重なる
	CHECK(Intersects(C(0, 0, 1), C(2, 0, 1)));      // ちょうど接する
	CHECK(!Intersects(C(0, 0, 1), C(2.01f, 0, 1))); // 離れている
	CHECK(Intersects(C(0, 0, 5), C(1, 0, 1)));      // 完全に内包
	CHECK(Intersects(C(0, 0, 1), C(0, 0, 1)));      // 同じ位置

	Contact2D contact = Collide(C(0, 0, 1), C(1.5f, 0, 1));
	CHECK(contact.hit);
	CHECK(Near(contact.normal, V(-1, 0)));          // 第1引数(原点側)を、第2引数から遠ざける向き
	CHECK_NEAR(contact.depth, 0.5, 1e-5);

	CHECK(!Collide(C(0, 0, 1), C(3, 0, 1)).hit);

	Contact2D same = Collide(C(2, 2, 1), C(2, 2, 1)); // 中心が一致: 向きは決まった値になり、NaNにならない
	CHECK(same.hit);
	CHECK_NEAR(Len(same.normal), 1.0, 1e-5);
	CHECK_NEAR(same.depth, 2.0, 1e-5);
}

TEST_CASE(Collision_CircleAabb) {
	const AABB2D box = R(0, 0, 2, 2);
	CHECK(Intersects(C(3, 1, 1.5f), box));     // 辺の外側から重なる
	CHECK(Intersects(C(3, 1, 1.0f), box));     // ちょうど接する
	CHECK(!Intersects(C(3, 1, 0.9f), box));
	CHECK(Intersects(C(1, 1, 0.1f), box));     // 中心が内側
	CHECK(Intersects(C(3, 3, 1.5f), box));     // 角: 距離√2 ≈ 1.414 < 1.5
	CHECK(!Intersects(C(3, 3, 1.4f), box));    // 角: 辺には届いても、角には届かない
	CHECK(Intersects(box, C(3, 1, 1.5f)));     // 引数の順を入れ替えても同じ

	// 辺の外側: 円を左へ押し戻す
	Contact2D side = Collide(C(3, 1, 1.5f), box);
	CHECK(side.hit);
	CHECK(Near(side.normal, V(1, 0)));
	CHECK_NEAR(side.depth, 0.5, 1e-5);

	// 角: 角から円の中心への向きへ押し出す
	Contact2D corner = Collide(C(3, 3, 1.5f), box);
	CHECK(corner.hit);
	CHECK(Near(corner.normal, V(0.70710678f, 0.70710678f), 1e-5f));
	CHECK_NEAR(corner.depth, 1.5 - std::sqrt(2.0), 1e-5);

	// 中心が内側: 最も近い面（右の面）から外へ。押し戻しの結果、円は右の面から半径ぶん離れる
	Contact2D inside = Collide(C(1.8f, 1, 0.5f), box);
	CHECK(inside.hit);
	CHECK(Near(inside.normal, V(1, 0)));
	CHECK_NEAR(inside.depth, 0.2 + 0.5, 1e-5);

	// 押し戻した位置では、ちょうど接する（めり込みがなくなる）
	Circle2D moved = C(1.8f + inside.normal.x * inside.depth, 1 + inside.normal.y * inside.depth, 0.5f);
	CHECK(!Collide(moved, box).hit || Collide(moved, box).depth < 1e-5f);

	// 矩形側から見ると、押し戻す向きは逆
	Contact2D reversed = Collide(box, C(3, 1, 1.5f));
	CHECK(Near(reversed.normal, V(-1, 0)));
	CHECK_NEAR(reversed.depth, 0.5, 1e-5);

	CHECK(!Collide(C(10, 10, 1), box).hit);
}

TEST_CASE(Collision_AabbAabb) {
	CHECK(Intersects(R(0, 0, 2, 2), R(1, 1, 3, 3)));    // 重なる
	CHECK(Intersects(R(0, 0, 2, 2), R(2, 0, 4, 2)));    // 辺で接する
	CHECK(Intersects(R(0, 0, 2, 2), R(2, 2, 4, 4)));    // 角で接する
	CHECK(!Intersects(R(0, 0, 2, 2), R(2.01f, 0, 4, 2)));
	CHECK(Intersects(R(0, 0, 10, 10), R(4, 4, 5, 5)));  // 完全に内包

	// X方向の重なりが浅い（0.5）ので、X方向（左）へ押し戻す
	Contact2D contact = Collide(R(0, 0, 2, 2), R(1.5f, 0, 4, 2));
	CHECK(contact.hit);
	CHECK(Near(contact.normal, V(-1, 0)));
	CHECK_NEAR(contact.depth, 0.5, 1e-5);

	// 上から乗っている（Y方向の重なりが浅い）: 上へ押し戻す
	Contact2D onTop = Collide(R(0, 1.8f, 2, 3.8f), R(0, 0, 2, 2));
	CHECK(onTop.hit);
	CHECK(Near(onTop.normal, V(0, 1)));
	CHECK_NEAR(onTop.depth, 0.2, 1e-5);

	CHECK(!Collide(R(0, 0, 1, 1), R(5, 5, 6, 6)).hit);
}

TEST_CASE(Collision_SegmentCircle) {
	CHECK(Intersects(S(-2, 0, 2, 0), C(0, 0.5f, 1)));    // 横切る
	CHECK(!Intersects(S(-2, 0, 2, 0), C(0, 2, 1)));
	CHECK(Intersects(S(-2, 0, 2, 0), C(0, 1, 1)));       // ちょうど接する
	CHECK(Intersects(S(-1, 0, 1, 0), C(0, 0, 5)));       // 線分が円に内包される
	CHECK(!Intersects(S(3, 0, 5, 0), C(0, 0, 1)));       // 線分の延長線上にあるが、届かない
	CHECK(Intersects(S(0.5f, 0, 0.5f, 0), C(0, 0, 1)));  // 長さ0の線分（点）が内側
	CHECK(!Intersects(S(3, 0, 3, 0), C(0, 0, 1)));       // 長さ0の線分（点）が外側
	CHECK(Intersects(C(0, 0.5f, 1), S(-2, 0, 2, 0)));    // 引数の順
}

TEST_CASE(Collision_RaycastCircle) {
	// 速い弾: 1フレームで円の反対側まで進んでも、最初に当たる位置が取れる（すり抜けない）
	RayHit2D hit = Raycast(S(-10, 0, 10, 0), C(0, 0, 1));
	CHECK(hit.hit);
	CHECK_NEAR(hit.t, 0.45, 1e-5);                      // x=-1 で当たる（全体20のうち9）
	CHECK(Near(hit.point, V(-1, 0)));
	CHECK(Near(hit.normal, V(-1, 0)));

	CHECK(!Raycast(S(-10, 3, 10, 3), C(0, 0, 1)).hit);   // 外れる
	CHECK(!Raycast(S(-10, 0, -5, 0), C(0, 0, 1)).hit);   // 届かない
	CHECK(!Raycast(S(10, 0, 5, 0), C(0, 0, 1)).hit);     // 向きが逆（円から遠ざかる）

	RayHit2D startInside = Raycast(S(0.2f, 0, 5, 0), C(0, 0, 1)); // 始点がすでに内側
	CHECK(startInside.hit);
	CHECK(startInside.t == 0.0f);

	RayHit2D graze = Raycast(S(-5, 1, 5, 1), C(0, 0, 1));         // ちょうど接する
	CHECK(graze.hit);

	CHECK(!Raycast(S(4, 0, 4, 0), C(0, 0, 1)).hit);               // 長さ0・外側
	CHECK(Raycast(S(0, 0, 0, 0), C(0, 0, 1)).hit);                // 長さ0・内側
}

TEST_CASE(Collision_RaycastAabb) {
	const AABB2D box = R(-1, -1, 1, 1);

	// 左から入る: 法線は -X
	RayHit2D left = Raycast(S(-10, 0, 10, 0), box);
	CHECK(left.hit);
	CHECK_NEAR(left.t, 0.45, 1e-5);
	CHECK(Near(left.point, V(-1, 0)));
	CHECK(Near(left.normal, V(-1, 0)));

	// 右から入る: 法線は +X
	RayHit2D right = Raycast(S(10, 0, -10, 0), box);
	CHECK(right.hit);
	CHECK(Near(right.normal, V(1, 0)));

	// 上（+Y）から入る / 下から入る
	CHECK(Near(Raycast(S(0, 10, 0, -10), box).normal, V(0, 1)));
	CHECK(Near(Raycast(S(0, -10, 0, 10), box).normal, V(0, -1)));

	// 斜めに角を通る
	RayHit2D diagonal = Raycast(S(-3, -3, 3, 3), box);
	CHECK(diagonal.hit);
	CHECK(Near(diagonal.point, V(-1, -1)));

	// 外れる・届かない・向きが逆
	CHECK(!Raycast(S(-10, 5, 10, 5), box).hit);
	CHECK(!Raycast(S(-10, 0, -5, 0), box).hit);
	CHECK(!Raycast(S(10, 0, 5, 0), box).hit);
	CHECK(!Raycast(S(0, 5, 5, 5), box).hit);

	// 軸に平行（dir=0）で、範囲の外 / 内を通る
	CHECK(!Raycast(S(-10, 1.5f, 10, 1.5f), box).hit);
	CHECK(Raycast(S(-10, 1.0f, 10, 1.0f), box).hit); // 辺の上（接する）

	// 始点が内側
	RayHit2D inside = Raycast(S(0, 0, 5, 0), box);
	CHECK(inside.hit);
	CHECK(inside.t == 0.0f);
	CHECK(Near(inside.normal, V(0, 0)));

	// 長さ0の線分（点）
	CHECK(Raycast(S(0, 0, 0, 0), box).hit);
	CHECK(!Raycast(S(5, 5, 5, 5), box).hit);

	// Intersects と整合する
	CHECK(Intersects(S(-10, 0, 10, 0), box));
	CHECK(!Intersects(S(-10, 5, 10, 5), box));
	CHECK(Intersects(box, S(-10, 0, 10, 0)));
}

TEST_CASE(Collision_SegmentSegment) {
	CHECK(Intersects(S(0, 0, 2, 2), S(0, 2, 2, 0)));         // X字に交差
	CHECK(!Intersects(S(0, 0, 1, 1), S(2, 0, 3, 1)));        // 離れている
	CHECK(Intersects(S(0, 0, 2, 0), S(1, 0, 1, 2)));         // T字（端が線上）
	CHECK(Intersects(S(0, 0, 1, 1), S(1, 1, 2, 0)));         // 端点どうしが一致
	CHECK(Intersects(S(0, 0, 4, 0), S(2, 0, 6, 0)));         // 同一直線上で重なる
	CHECK(!Intersects(S(0, 0, 1, 0), S(2, 0, 3, 0)));        // 同一直線上で離れている
	CHECK(!Intersects(S(0, 0, 4, 0), S(0, 1, 4, 1)));        // 平行
	CHECK(Intersects(S(1, 1, 1, 1), S(0, 0, 2, 2)));         // 点が線分の上
	CHECK(!Intersects(S(1, 2, 1, 2), S(0, 0, 2, 2)));        // 点が線分の外
}

TEST_CASE(Collision_ResolveByContact) {
	// 押し戻しの手順: 円どうしがめり込んだら、normal*depth だけ動かして接する状態にする
	Circle2D a = C(0.5f, 0, 1);
	Circle2D b = C(0, 0, 1);
	Contact2D contact = Collide(a, b);
	CHECK(contact.hit);
	a.center = a.center + contact.normal * contact.depth;
	CHECK_NEAR(Len(a.center - b.center), 2.0, 1e-5);
	CHECK(Intersects(a, b)); // 接している状態は、当たっている扱い
	CHECK_NEAR(Collide(a, b).depth, 0.0, 1e-5);
}

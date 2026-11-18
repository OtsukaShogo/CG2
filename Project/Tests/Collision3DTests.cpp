#include "TestFramework.h"
#include "Source/Engine/Collision/Collision3D.h"
#include "Source/Engine/Math/MathUtil.h"
#include "Source/Engine/Math/TransformMatrix.h"

#include <cmath>
#include <cstdint>

using namespace Engine;

namespace {

Vector3 V(float x, float y, float z) { return Vector3{ x, y, z }; }
Sphere Sph(float x, float y, float z, float r) { return Sphere{ V(x, y, z), r }; }
AABB Box(float x0, float y0, float z0, float x1, float y1, float z1) { return AABB{ V(x0, y0, z0), V(x1, y1, z1) }; }
Segment3D Seg(float x0, float y0, float z0, float x1, float y1, float z1) { return Segment3D{ V(x0, y0, z0), V(x1, y1, z1) }; }
Capsule Cap(const Vector3& a, const Vector3& b, float r) { return Capsule{ Segment3D{ a, b }, r }; }
Ray MakeRay(const Vector3& origin, const Vector3& direction) { return Ray{ origin, Normalize(direction) }; }

bool Near(const Vector3& a, const Vector3& b, float eps = 1e-4f) { return NearlyEqual(a, b, eps); }

// 再現性のある乱数（テストごとに同じ結果になる）
struct Random {
	uint32_t state;
	explicit Random(uint32_t seed) : state(seed) {}
	float Next() { // 0〜1
		state = state * 1664525u + 1013904223u;
		return static_cast<float>((state >> 8) & 0xFFFF) / 65535.0f;
	}
	float Range(float lo, float hi) { return lo + (hi - lo) * Next(); }
	Vector3 Point(float extent) { return V(Range(-extent, extent), Range(-extent, extent), Range(-extent, extent)); }
	Vector3 Direction() {
		for (;;) {
			Vector3 v = Point(1.0f);
			if (Dot(v, v) > 0.01f && Dot(v, v) <= 1.0f) return Normalize(v);
		}
	}
};

OBB RandomOBB(Random& rng, float centerExtent) {
	return MakeOBB(rng.Point(centerExtent), V(rng.Range(0.3f, 1.5f), rng.Range(0.3f, 1.5f), rng.Range(0.3f, 1.5f)),
		V(rng.Range(-3.0f, 3.0f), rng.Range(-3.0f, 3.0f), rng.Range(-3.0f, 3.0f)));
}

// 形を平行移動する
Sphere Moved(Sphere s, const Vector3& d) { s.center += d; return s; }
Capsule Moved(Capsule c, const Vector3& d) { c.segment.start += d; c.segment.end += d; return c; }
OBB Moved(OBB o, const Vector3& d) { o.center += d; return o; }
AABB Moved(AABB a, const Vector3& d) { a.min += d; a.max += d; return a; }

// 点が「箱の表面上」にあるか（箱の内側を少し縮めた範囲には入らず、少し広げた範囲には入る）
bool OnSurface(const OBB& obb, const Vector3& p, float eps = 1e-3f) {
	OBB inner = obb, outer = obb;
	inner.halfExtents = obb.halfExtents - V(eps, eps, eps);
	outer.halfExtents = obb.halfExtents + V(eps, eps, eps);
	return Contains(outer, p) && !Contains(inner, p);
}

} // namespace

// ---- 形の作成 ----

TEST_CASE(Collision3D_MakeOBBMatchesModelTransform) {
	// 見た目の Model（MakeAffineMatrix）と、同じ位置・向きの判定になる
	Vector3 scale = V(2, 4, 6), euler = V(0.4f, -0.9f, 1.3f), translate = V(5, -6, 7);
	Matrix4x4 world = MakeAffineMatrix(scale, euler, translate);
	OBB obb = MakeOBB(translate, V(1, 2, 3), euler); // 大きさ1の立方体を scale 倍 → 半分の長さは scale/2

	Vector3 corners[8];
	GetCorners(obb, corners);
	for (int i = 0; i < 8; ++i) {
		Vector3 local = V((i & 1) ? 0.5f : -0.5f, (i & 2) ? 0.5f : -0.5f, (i & 4) ? 0.5f : -0.5f);
		CHECK(Near(corners[i], Transform(local, world), 1e-3f));
	}
	// 軸は互いに直交する長さ1
	CHECK_NEAR(Length(obb.axes[0]), 1.0, 1e-5);
	CHECK_NEAR(Dot(obb.axes[0], obb.axes[1]), 0.0, 1e-5);
	CHECK_NEAR(Dot(obb.axes[1], obb.axes[2]), 0.0, 1e-5);
}

TEST_CASE(Collision3D_MakeHelpers) {
	OBB fromAabb = ToOBB(Box(-1, -2, -3, 3, 2, 5));
	CHECK(Near(fromAabb.center, V(1, 0, 1)));
	CHECK(Near(fromAabb.halfExtents, V(2, 2, 4)));

	Capsule upright = MakeUprightCapsule(V(1, 0, 2), 2.0f, 0.5f);
	CHECK(Near(upright.segment.start, V(1, 0.5f, 2)));
	CHECK(Near(upright.segment.end, V(1, 1.5f, 2)));
	Capsule squat = MakeUprightCapsule(V(0, 0, 0), 0.8f, 0.5f); // 高さが直径より小さい: 球
	CHECK(Near(squat.segment.start, squat.segment.end));

	Plane plane = MakePlane(V(0, 3, 0), V(0, 5, 0));
	CHECK(Near(plane.normal, V(0, 1, 0)));
	CHECK_NEAR(plane.distance, 3.0, 1e-5);
	CHECK(Near(MakePlane(V(0, 0, 0), V(0, 0, 0)).normal, V(0, 1, 0))); // ゼロ法線は上向き
}

// ---- 最近接点 ----

TEST_CASE(Collision3D_ClosestPoints) {
	CHECK(Near(ClosestPoint(V(5, 5, 5), Box(0, 0, 0, 2, 2, 2)), V(2, 2, 2)));
	CHECK(Near(ClosestPoint(V(1, 1, 1), Box(0, 0, 0, 2, 2, 2)), V(1, 1, 1)));
	CHECK(Near(ClosestPoint(V(5, 3, 0), Seg(0, 0, 0, 10, 0, 0)), V(5, 0, 0)));
	CHECK(Near(ClosestPoint(V(-5, 3, 0), Seg(0, 0, 0, 10, 0, 0)), V(0, 0, 0)));
	CHECK(Near(ClosestPoint(V(3, 4, 5), Seg(1, 1, 1, 1, 1, 1)), V(1, 1, 1))); // 長さ0の線分でもNaNにならない

	// 回転した箱: 45度回した箱の角の外側
	OBB rotated = MakeOBB(V(0, 0, 0), V(1, 1, 1), V(0, kPi * 0.25f, 0));
	Vector3 closest = ClosestPoint(V(0, 0, 3), rotated);
	CHECK(Near(closest, V(0, 0, 1.41421356f), 1e-4f)); // 回した角（対角の半分 √2）
	CHECK(Near(ClosestPoint(V(0.1f, 0.2f, 0.1f), rotated), V(0.1f, 0.2f, 0.1f))); // 内側
}

TEST_CASE(Collision3D_SegmentSegmentClosest) {
	Vector3 a, b;
	// ねじれの位置の2本: X軸と、高さ2を通るZ軸方向の線 → 距離2
	CHECK_NEAR(ClosestPoints(Seg(-5, 0, 0, 5, 0, 0), Seg(1, 2, -5, 1, 2, 5), &a, &b), 4.0, 1e-4);
	CHECK(Near(a, V(1, 0, 0)));
	CHECK(Near(b, V(1, 2, 0)));
	// 平行
	CHECK_NEAR(ClosestPoints(Seg(0, 0, 0, 10, 0, 0), Seg(2, 3, 0, 8, 3, 0), &a, &b), 9.0, 1e-4);
	// 端点どうしが最近
	CHECK_NEAR(ClosestPoints(Seg(0, 0, 0, 1, 0, 0), Seg(3, 0, 0, 4, 1, 0), &a, &b), 4.0, 1e-4);
	// 交差
	CHECK_NEAR(ClosestPoints(Seg(-1, 0, 0, 1, 0, 0), Seg(0, -1, 0, 0, 1, 0), &a, &b), 0.0, 1e-6);
	// 点と線分 / 点と点（長さ0）
	CHECK_NEAR(ClosestPoints(Seg(0, 0, 0, 0, 0, 0), Seg(3, 4, 0, 3, 4, 0), &a, &b), 25.0, 1e-4);
	CHECK_NEAR(ClosestPoints(Seg(0, 0, 0, 0, 0, 0), Seg(-5, 3, 0, 5, 3, 0), &a, &b), 9.0, 1e-4);
	CHECK(Near(b, V(0, 3, 0)));
}

TEST_CASE(Collision3D_SegmentBoxDistanceMatchesSampling) {
	// 線分と箱の距離を、線分上に細かく並べた点と箱の距離の最小値（総当たり）と比べる
	Random rng(1234);
	int checked = 0;
	for (int n = 0; n < 150; ++n) {
		OBB obb = RandomOBB(rng, 2.0f);
		Segment3D seg{ rng.Point(5.0f), rng.Point(5.0f) };
		Vector3 onSegment, onBox;
		float exact = std::sqrt(ClosestPoints(seg, obb, &onSegment, &onBox));

		float sampled = 1e9f;
		const int steps = 4000;
		for (int i = 0; i <= steps; ++i) {
			Vector3 p = seg.start + (seg.end - seg.start) * (static_cast<float>(i) / steps);
			sampled = (std::min)(sampled, Length(Vector3{ p - ClosestPoint(p, obb) }));
		}
		// 総当たりは、間隔のぶんだけ大きめに出る。厳密値は、それ以下で、差は間隔程度
		CHECK(exact <= sampled + 1e-3f);
		CHECK(sampled - exact < Length(Vector3{ seg.end - seg.start }) / 4000.0f + 2e-3f);
		// 返した2点は、実際にそれぞれの形の上にある
		CHECK(Near(ClosestPoint(onBox, obb), onBox, 1e-3f));
		CHECK_NEAR(Length(Vector3{ onSegment - onBox }), exact, 1e-3);
		++checked;
	}
	CHECK(checked == 150);
}

// ---- 判定のみ・押し戻し（個別の境界値） ----

TEST_CASE(Collision3D_SphereSphere) {
	CHECK(Intersects(Sph(0, 0, 0, 1), Sph(1.5f, 0, 0, 1)));
	CHECK(Intersects(Sph(0, 0, 0, 1), Sph(2, 0, 0, 1)));      // ちょうど接する
	CHECK(!Intersects(Sph(0, 0, 0, 1), Sph(2.01f, 0, 0, 1)));
	CHECK(Intersects(Sph(0, 0, 0, 5), Sph(1, 0, 0, 1)));      // 内包

	Contact3D c = Collide(Sph(0, 0, 0, 1), Sph(1.5f, 0, 0, 1));
	CHECK(c.hit);
	CHECK(Near(c.normal, V(-1, 0, 0)));
	CHECK_NEAR(c.depth, 0.5, 1e-5);
	Contact3D same = Collide(Sph(1, 1, 1, 1), Sph(1, 1, 1, 1)); // 中心が一致
	CHECK(same.hit);
	CHECK_NEAR(Length(same.normal), 1.0, 1e-5);
	CHECK(!Collide(Sph(0, 0, 0, 1), Sph(5, 0, 0, 1)).hit);
}

TEST_CASE(Collision3D_SphereBox) {
	const AABB box = Box(0, 0, 0, 2, 2, 2);
	CHECK(Intersects(Sph(3, 1, 1, 1.5f), box));
	CHECK(Intersects(Sph(3, 1, 1, 1.0f), box));       // 接する
	CHECK(!Intersects(Sph(3, 1, 1, 0.9f), box));
	CHECK(!Intersects(Sph(3, 3, 3, 1.5f), box));      // 角: 距離√3 ≈ 1.732
	CHECK(Intersects(Sph(3, 3, 3, 1.8f), box));
	CHECK(Intersects(box, Sph(3, 1, 1, 1.5f)));       // 引数の順

	Contact3D side = Collide(Sph(3, 1, 1, 1.5f), box);
	CHECK(side.hit);
	CHECK(Near(side.normal, V(1, 0, 0)));
	CHECK_NEAR(side.depth, 0.5, 1e-5);

	// 中心が内側: 最も近い面（+X側）から押し出す
	Contact3D inside = Collide(Sph(1.8f, 1, 1, 0.5f), box);
	CHECK(inside.hit);
	CHECK(Near(inside.normal, V(1, 0, 0)));
	CHECK_NEAR(inside.depth, 0.7, 1e-5);

	// 箱側から見ると逆向き
	CHECK(Near(Collide(box, Sph(3, 1, 1, 1.5f)).normal, V(-1, 0, 0)));

	// 回転した箱
	OBB rotated = MakeOBB(V(0, 0, 0), V(1, 1, 1), V(0, kPi * 0.25f, 0));
	CHECK(Intersects(Sph(0, 0, 2, 0.7f), rotated));   // 角（√2≈1.414）に届く
	CHECK(!Intersects(Sph(0, 0, 2, 0.5f), rotated));
	CHECK(Intersects(rotated, Sph(0, 0, 2, 0.7f)));
	CHECK(!Collide(Sph(0, 0, 5, 1), rotated).hit);
}

TEST_CASE(Collision3D_BoxBox) {
	CHECK(Intersects(Box(0, 0, 0, 2, 2, 2), Box(1, 1, 1, 3, 3, 3)));
	CHECK(Intersects(Box(0, 0, 0, 2, 2, 2), Box(2, 0, 0, 4, 2, 2)));        // 面で接する
	CHECK(!Intersects(Box(0, 0, 0, 2, 2, 2), Box(2.01f, 0, 0, 4, 2, 2)));
	CHECK(Intersects(Box(0, 0, 0, 10, 10, 10), Box(4, 4, 4, 5, 5, 5)));     // 内包

	Contact3D c = Collide(Box(0, 0, 0, 2, 2, 2), Box(1.5f, 0, 0, 4, 2, 2)); // Xの重なりが最も浅い
	CHECK(c.hit);
	CHECK(Near(c.normal, V(-1, 0, 0)));
	CHECK_NEAR(c.depth, 0.5, 1e-5);
	Contact3D onTop = Collide(Box(0, 1.8f, 0, 2, 3.8f, 2), Box(0, 0, 0, 2, 2, 2)); // 上に乗っている
	CHECK(Near(onTop.normal, V(0, 1, 0)));
	CHECK_NEAR(onTop.depth, 0.2, 1e-5);
}

TEST_CASE(Collision3D_ObbObb) {
	OBB a = MakeOBB(V(0, 0, 0), V(1, 1, 1), V(0, 0, 0));
	OBB rotated = MakeOBB(V(2.2f, 0, 0), V(1, 1, 1), V(0, kPi * 0.25f, 0)); // 45度: 角が √2 ≈ 1.414 まで伸びる
	CHECK(Intersects(a, rotated));                    // 1 + 1.414 = 2.414 > 2.2
	CHECK(!Intersects(a, Moved(rotated, V(0.3f, 0, 0))));
	CHECK(Intersects(a, a));                          // 同じ箱
	CHECK(Intersects(ToOBB(Box(-1, -1, -1, 1, 1, 1)), rotated)); // AABB × OBB
	CHECK(Intersects(rotated, ToOBB(Box(-1, -1, -1, 1, 1, 1))));

	Contact3D c = Collide(a, rotated);
	CHECK(c.hit);
	CHECK(c.normal.x < -0.9f);                        // aを、-X（rotatedから遠ざかる側）へ
	CHECK_NEAR(c.depth, 1.0 + std::sqrt(2.0) - 2.2, 2e-3); // aの半分(1) + 回した箱のX方向の張り出し(√2) - 中心間の距離(2.2)

	// 平行に近い軸（辺どうしの軸が定まらない）でも、NaNにならない
	OBB tilted = MakeOBB(V(0, 0, 1.5f), V(1, 1, 1), V(1e-4f, 0, 0));
	Contact3D nearly = Collide(a, tilted);
	CHECK(nearly.hit);
	CHECK(std::isfinite(nearly.depth));
	CHECK_NEAR(Length(nearly.normal), 1.0, 1e-4);
}

TEST_CASE(Collision3D_Capsule) {
	// カプセルとカプセル（立っている2人）
	Capsule a = MakeUprightCapsule(V(0, 0, 0), 2.0f, 0.5f);
	Capsule b = MakeUprightCapsule(V(0.8f, 0, 0), 2.0f, 0.5f);
	CHECK(Intersects(a, b));
	CHECK(!Intersects(a, Moved(b, V(0.3f, 0, 0))));
	CHECK(Intersects(a, Moved(b, V(0.2f, 0, 0)))); // 距離 1.0 = 半径の和: 接する
	Contact3D c = Collide(a, b);
	CHECK(c.hit);
	CHECK(Near(c.normal, V(-1, 0, 0)));
	CHECK_NEAR(c.depth, 0.2, 1e-5);

	// 軸が交差する（十字）: 垂直な向きへ押し出す
	Capsule cross1 = Cap(V(-2, 0, 0), V(2, 0, 0), 0.5f);
	Capsule cross2 = Cap(V(0, -2, 0), V(0, 2, 0), 0.5f);
	Contact3D crossed = Collide(cross1, cross2);
	CHECK(crossed.hit);
	CHECK_NEAR(Length(crossed.normal), 1.0, 1e-5);
	CHECK_NEAR(std::abs(crossed.normal.z), 1.0, 1e-5);
	CHECK_NEAR(crossed.depth, 1.0, 1e-5);

	// 球とカプセル（側面・端の半球）
	CHECK(Intersects(Sph(1, 0.9f, 0, 0.5f), a));  // 側面
}

TEST_CASE(Collision3D_CapsuleSphereDetails) {
	Capsule a = MakeUprightCapsule(V(0, 0, 0), 2.0f, 0.5f); // 軸は y=0.5〜1.5
	CHECK(!Intersects(Sph(0, 2.9f, 0, 0.5f), a));          // 軸の端から1.4: 半径の和1.0より遠い
	CHECK(Intersects(Sph(0, 2.4f, 0, 0.5f), a));           // 軸の端から0.9: 当たる
	CHECK(Intersects(Sph(0.9f, 1.0f, 0, 0.4f), a));        // 側面: 距離0.9 = 半径の和 → 接する
	Contact3D c = Collide(Sph(0.8f, 1.0f, 0, 0.5f), a);
	CHECK(c.hit);
	CHECK(Near(c.normal, V(1, 0, 0)));
	CHECK_NEAR(c.depth, 0.2, 1e-5);
	CHECK(Near(Collide(a, Sph(0.8f, 1.0f, 0, 0.5f)).normal, V(-1, 0, 0)));
}

TEST_CASE(Collision3D_CapsuleBox) {
	const AABB wall = Box(2, 0, -5, 3, 4, 5);
	Capsule player = MakeUprightCapsule(V(1.2f, 0, 0), 2.0f, 0.5f); // 軸のxは1.2: 壁(x=2)まで0.8、半径0.5
	CHECK(!Intersects(player, wall));
	player = MakeUprightCapsule(V(1.7f, 0, 0), 2.0f, 0.5f);
	CHECK(Intersects(player, wall));
	Contact3D c = Collide(player, wall);
	CHECK(c.hit);
	CHECK(Near(c.normal, V(-1, 0, 0)));                          // 壁から離れる向き
	CHECK_NEAR(c.depth, 0.2, 1e-4);

	// 押し戻した位置は、壁にちょうど接する
	Capsule moved = Moved(player, c.normal * c.depth);
	CHECK_NEAR(Collide(moved, wall).depth, 0.0, 1e-3);

	// 床（箱）の上に立つ: 足元が少しめり込んでいる → 上へ
	const AABB floor = Box(-10, -1, -10, 10, 0, 10);
	Capsule standing = MakeUprightCapsule(V(0, -0.1f, 0), 2.0f, 0.5f);
	Contact3D ground = Collide(standing, floor);
	CHECK(ground.hit);
	CHECK(Near(ground.normal, V(0, 1, 0)));
	CHECK_NEAR(ground.depth, 0.1, 1e-4);

	// 軸が箱に食い込んでいる場合（箱が細い柱）でも、有限の向きと深さを返す
	Contact3D pillar = Collide(Cap(V(0, 0, 0), V(0, 3, 0), 0.5f), Box(-0.2f, 1, -0.2f, 0.2f, 2, 0.2f));
	CHECK(pillar.hit);
	CHECK_NEAR(Length(pillar.normal), 1.0, 1e-4);
	CHECK(std::isfinite(pillar.depth));
}

TEST_CASE(Collision3D_Plane) {
	Plane ground = MakePlane(V(0, 0, 0), V(0, 1, 0));
	CHECK(Intersects(Sph(0, 0.5f, 0, 0.5f), ground));  // 接する
	CHECK(!Intersects(Sph(0, 0.6f, 0, 0.5f), ground));
	CHECK(Intersects(Plane{ ground }, Sph(0, -0.3f, 0, 0.5f)));
	Contact3D c = Collide(Sph(0, 0.3f, 0, 0.5f), ground);
	CHECK(c.hit);
	CHECK(Near(c.normal, V(0, 1, 0)));
	CHECK_NEAR(c.depth, 0.2, 1e-5);
	Contact3D below = Collide(Sph(0, -0.3f, 0, 0.5f), ground); // 平面の裏側: 裏へ押し出す
	CHECK(Near(below.normal, V(0, -1, 0)));

	// カプセル: 足元が少しめり込む → 上へ
	Capsule standing = MakeUprightCapsule(V(0, -0.1f, 0), 2.0f, 0.5f);
	Contact3D foot = Collide(standing, ground);
	CHECK(foot.hit);
	CHECK(Near(foot.normal, V(0, 1, 0)));
	CHECK_NEAR(foot.depth, 0.1, 1e-5);
	CHECK(!Collide(MakeUprightCapsule(V(0, 0.1f, 0), 2.0f, 0.5f), ground).hit);
	// 軸が平面を跨ぐ（横に倒れて沈んだ）: 中点のある側へ押し出す
	Contact3D lying = Collide(Cap(V(-1, -0.1f, 0), V(1, 0.3f, 0), 0.5f), ground);
	CHECK(lying.hit);
	CHECK(Near(lying.normal, V(0, 1, 0)));
	CHECK_NEAR(lying.depth, 0.5 + 0.1, 1e-5);

	CHECK(Intersects(Box(-1, -1, -1, 1, 1, 1), ground));
	CHECK(!Intersects(Box(-1, 1.01f, -1, 1, 2, 1), ground));
	CHECK(Intersects(MakeOBB(V(0, 1.2f, 0), V(1, 1, 1), V(0, 0, kPi * 0.25f)), ground)); // 傾いた箱は、角が下に出る

	// 線分
	CHECK(Intersects(Seg(0, -1, 0, 0, 1, 0), ground));
	CHECK(!Intersects(Seg(0, 1, 0, 0, 2, 0), ground));
}

// ---- レイキャスト ----

TEST_CASE(Collision3D_RaycastSphere) {
	RayHit3D hit = Raycast(Seg(-10, 0, 0, 10, 0, 0), Sph(0, 0, 0, 1)); // 速い弾: 反対側へ抜ける線分でも拾う
	CHECK(hit.hit);
	CHECK_NEAR(hit.t, 0.45, 1e-5);
	CHECK(Near(hit.point, V(-1, 0, 0)));
	CHECK(Near(hit.normal, V(-1, 0, 0)));
	CHECK(!Raycast(Seg(-10, 3, 0, 10, 3, 0), Sph(0, 0, 0, 1)).hit);
	CHECK(!Raycast(Seg(10, 0, 0, 5, 0, 0), Sph(0, 0, 0, 1)).hit);       // 遠ざかる
	CHECK(Raycast(Seg(0.2f, 0, 0, 5, 0, 0), Sph(0, 0, 0, 1)).t == 0.0f); // 始点が内側
	CHECK(!Raycast(Seg(4, 0, 0, 4, 0, 0), Sph(0, 0, 0, 1)).hit);         // 長さ0

	// レイ: t は距離
	RayHit3D ray = Raycast(MakeRay(V(0, 0, -10), V(0, 0, 1)), Sph(0, 0, 0, 1));
	CHECK(ray.hit);
	CHECK_NEAR(ray.t, 9.0, 1e-4);
	CHECK(!Raycast(MakeRay(V(0, 0, -10), V(0, 0, 1)), Sph(0, 0, 0, 1), 5.0f).hit); // 最大距離で打ち切る
	CHECK(!Raycast(MakeRay(V(0, 0, -10), V(0, 0, -1)), Sph(0, 0, 0, 1)).hit);       // 後ろ向き
}

TEST_CASE(Collision3D_RaycastBoxes) {
	const AABB box = Box(-1, -1, -1, 1, 1, 1);
	RayHit3D left = Raycast(Seg(-10, 0, 0, 10, 0, 0), box);
	CHECK(left.hit);
	CHECK_NEAR(left.t, 0.45, 1e-5);
	CHECK(Near(left.normal, V(-1, 0, 0)));
	CHECK(Near(Raycast(Seg(10, 0, 0, -10, 0, 0), box).normal, V(1, 0, 0)));
	CHECK(Near(Raycast(Seg(0, 10, 0, 0, -10, 0), box).normal, V(0, 1, 0)));
	CHECK(Near(Raycast(Seg(0, 0, -10, 0, 0, 10), box).normal, V(0, 0, -1)));
	CHECK(!Raycast(Seg(-10, 5, 0, 10, 5, 0), box).hit);
	CHECK(!Raycast(Seg(-10, 0, 0, -5, 0, 0), box).hit);
	CHECK(!Raycast(Seg(10, 0, 0, 5, 0, 0), box).hit);
	CHECK(Raycast(Seg(-10, 1.0f, 0, 10, 1.0f, 0), box).hit); // 面に接する
	RayHit3D inside = Raycast(Seg(0, 0, 0, 5, 0, 0), box);
	CHECK(inside.hit);
	CHECK(inside.t == 0.0f);
	CHECK(Near(inside.normal, V(0, 0, 0)));

	// 回転した箱は、回した分だけ、当たる位置が変わる（45度回した箱は、角が±√2 まで）
	OBB rotated = MakeOBB(V(0, 0, 0), V(1, 1, 1), V(0, kPi * 0.25f, 0));
	RayHit3D diag = Raycast(MakeRay(V(-10, 0, 0), V(1, 0, 0)), rotated);
	CHECK(diag.hit);
	CHECK_NEAR(diag.t, 10.0 - std::sqrt(2.0), 1e-3);
	CHECK(Near(diag.point, V(-1.41421356f, 0, 0), 1e-3f));
	// 軸に平行な向きの OBB は、AABB と同じ結果
	OBB same = ToOBB(box);
	RayHit3D a = Raycast(Seg(-10, 0.3f, 0.2f, 10, -0.1f, 0.4f), box);
	RayHit3D b = Raycast(Seg(-10, 0.3f, 0.2f, 10, -0.1f, 0.4f), same);
	CHECK(a.hit == b.hit);
	CHECK_NEAR(a.t, b.t, 1e-5);
	CHECK(Near(a.normal, b.normal));
}

TEST_CASE(Collision3D_RaycastCapsule) {
	const Capsule capsule = Cap(V(0, 0, 0), V(0, 2, 0), 0.5f);
	// 側面
	RayHit3D side = Raycast(Seg(-5, 1, 0, 5, 1, 0), capsule);
	CHECK(side.hit);
	CHECK_NEAR(side.t, 0.45, 1e-5);
	CHECK(Near(side.normal, V(-1, 0, 0)));
	// 上端の半球
	RayHit3D top = Raycast(Seg(0, 5, 0, 0, -5, 0), capsule);
	CHECK(top.hit);
	CHECK(Near(top.point, V(0, 2.5f, 0)));
	CHECK(Near(top.normal, V(0, 1, 0)));
	// 半球をかすめる / 外れる
	CHECK(Raycast(Seg(-5, 2.3f, 0, 5, 2.3f, 0), capsule).hit);
	CHECK(!Raycast(Seg(-5, 2.6f, 0, 5, 2.6f, 0), capsule).hit);
	CHECK(!Raycast(Seg(-5, 1, 2, 5, 1, 2), capsule).hit);
	// 軸に平行（真上から落ちる線）で、側面の外
	CHECK(!Raycast(Seg(1, 5, 0, 1, -5, 0), capsule).hit);
	// 始点が内側
	RayHit3D inside = Raycast(Seg(0, 1, 0, 5, 1, 0), capsule);
	CHECK(inside.hit);
	CHECK(inside.t == 0.0f);
	// 軸の長さ0のカプセルは、球と同じ
	RayHit3D asSphere = Raycast(Seg(-5, 0, 0, 5, 0, 0), Cap(V(0, 0, 0), V(0, 0, 0), 0.5f));
	CHECK(asSphere.hit);
	CHECK_NEAR(asSphere.t, 0.45, 1e-5);
}

TEST_CASE(Collision3D_RaycastPlane) {
	Plane ground = MakePlane(V(0, 0, 0), V(0, 1, 0));
	// マウス照準: 斜め上から地面へ向かうレイ
	RayHit3D hit = Raycast(MakeRay(V(0, 10, -10), V(0, -1, 1)), ground);
	CHECK(hit.hit);
	CHECK(Near(hit.point, V(0, 0, 0), 1e-4f));
	CHECK_NEAR(hit.t, 10.0 * std::sqrt(2.0), 1e-3);
	CHECK(Near(hit.normal, V(0, 1, 0)));
	CHECK(!Raycast(MakeRay(V(0, 10, 0), V(0, 1, 0)), ground).hit);   // 上へ向かう: 当たらない
	CHECK(!Raycast(MakeRay(V(0, 10, 0), V(1, 0, 0)), ground).hit);   // 平行
	CHECK(!Raycast(MakeRay(V(0, 10, 0), V(0, -1, 0)), ground, 5.0f).hit); // 最大距離
	// 裏側から来るレイ: 法線は、レイが来る側を向く
	CHECK(Near(Raycast(MakeRay(V(0, -3, 0), V(0, 1, 0)), ground).normal, V(0, -1, 0)));
	// 線分
	RayHit3D seg = Raycast(Seg(0, 4, 0, 0, -4, 0), ground);
	CHECK(seg.hit);
	CHECK_NEAR(seg.t, 0.5, 1e-5);
	// 傾いた平面
	Plane slope = MakePlane(V(0, 0, 0), V(1, 1, 0));
	CHECK(Raycast(MakeRay(V(5, 0, 0), V(-1, 0, 0)), slope).hit);
}

// ---- 乱数による検証（個別のケースでは見つけにくい誤りを探す） ----

TEST_CASE(Collision3D_Random_CollideAgreesWithIntersects) {
	Random rng(42);
	int strict = 0;
	for (int n = 0; n < 600; ++n) {
		Sphere s = Sph(rng.Range(-3, 3), rng.Range(-3, 3), rng.Range(-3, 3), rng.Range(0.2f, 1.5f));
		Capsule c = Cap(rng.Point(3.0f), rng.Point(3.0f), rng.Range(0.2f, 1.0f));
		OBB o = RandomOBB(rng, 3.0f);
		OBB o2 = RandomOBB(rng, 3.0f);
		AABB b = Box(-1, -1, -1, 1.5f, 1, 2);
		AABB b2 = Moved(b, rng.Point(2.5f));
		Capsule c2 = Cap(rng.Point(3.0f), rng.Point(3.0f), rng.Range(0.2f, 1.0f));
		strict += Intersects(s, c) != Collide(s, c).hit;
		strict += Intersects(s, o) != Collide(s, o).hit;
		strict += Intersects(s, b) != Collide(s, b).hit;
		strict += Intersects(c, c2) != Collide(c, c2).hit;
		strict += Intersects(c, o) != Collide(c, o).hit;
		strict += Intersects(c, b) != Collide(c, b).hit;
		strict += Intersects(o, o2) != Collide(o, o2).hit;
		strict += Intersects(b, o) != Collide(b, o).hit;
		strict += Intersects(b, b2) != Collide(b, b2).hit;
		// 引数の順を入れ替えても、結果は同じ
		strict += Intersects(o, s) != Intersects(s, o);
		strict += Intersects(o, c) != Intersects(c, o);
		strict += Intersects(o2, o) != Intersects(o, o2);
		strict += Collide(c2, c).hit != Collide(c, c2).hit;
	}
	CHECK(strict == 0);
}

TEST_CASE(Collision3D_Random_PushOutSeparates) {
	// 押し戻した位置では、もう重ならない（少し余裕を持って動かす）。組み合わせごとに数える
	enum { kSphereCapsule, kSphereObb, kSphereAabb, kCapsuleCapsule, kObbObb, kAabbObb, kCount };
	const char* names[kCount] = { "sphere-capsule", "sphere-obb", "sphere-aabb", "capsule-capsule", "obb-obb", "aabb-obb" };
	int tested[kCount] = {}, failed[kCount] = {};

	Random rng(7);
	for (int n = 0; n < 1500; ++n) {
		Sphere s = Sph(rng.Range(-2, 2), rng.Range(-2, 2), rng.Range(-2, 2), rng.Range(0.3f, 1.2f));
		Capsule c = Cap(rng.Point(2.0f), rng.Point(2.0f), rng.Range(0.3f, 0.8f));
		Capsule c2 = Cap(rng.Point(2.0f), rng.Point(2.0f), rng.Range(0.3f, 0.8f));
		OBB o = RandomOBB(rng, 1.5f);
		OBB o2 = RandomOBB(rng, 1.5f);
		AABB b = Moved(Box(-1, -1, -1, 1, 1, 1), rng.Point(1.5f));

		auto check = [&](int kind, const Contact3D& contact, bool stillOverlapping) {
			if (!contact.hit) return;
			++tested[kind];
			failed[kind] += stillOverlapping;
		};
		{ Contact3D k = Collide(s, c);  check(kSphereCapsule, k, Intersects(Moved(s, k.normal * (k.depth + 1e-3f)), c)); }
		{ Contact3D k = Collide(s, o);  check(kSphereObb, k, Intersects(Moved(s, k.normal * (k.depth + 1e-3f)), o)); }
		{ Contact3D k = Collide(s, b);  check(kSphereAabb, k, Intersects(Moved(s, k.normal * (k.depth + 1e-3f)), b)); }
		{ Contact3D k = Collide(c, c2); check(kCapsuleCapsule, k, Intersects(Moved(c, k.normal * (k.depth + 1e-3f)), c2)); }
		{ Contact3D k = Collide(o, o2); check(kObbObb, k, Intersects(Moved(o, k.normal * (k.depth + 1e-3f)), o2)); }
		{ Contact3D k = Collide(b, o);  check(kAabbObb, k, Intersects(Moved(b, k.normal * (k.depth + 1e-3f)), o)); }
	}
	for (int i = 0; i < kCount; ++i) {
		std::printf("    %-16s tested=%d failed=%d\n", names[i], tested[i], failed[i]);
		CHECK(tested[i] > 50); // 十分な数の「当たっているケース」を試している
		CHECK(failed[i] == 0);
	}
}

TEST_CASE(Collision3D_Random_CapsuleBoxPushOutSeparatesInOneStep) {
	// カプセルと箱: 軸が箱に食い込んでいる場合（分離軸で押し出す）も含めて、1回の押し戻しで離れる
	Random rng(99);
	int tested = 0, deep = 0, stuck = 0;
	for (int n = 0; n < 1500; ++n) {
		Capsule c = Cap(rng.Point(2.0f), rng.Point(2.0f), rng.Range(0.3f, 0.8f));
		OBB o = RandomOBB(rng, 1.5f);
		Contact3D contact = Collide(c, o);
		if (!contact.hit) continue;
		++tested;
		Vector3 a, b;
		deep += ClosestPoints(c.segment, o, &a, &b) == 0.0f; // 軸が箱と交差している
		stuck += Intersects(Moved(c, contact.normal * (contact.depth + 1e-3f)), o);

		// AABB でも同じ
		AABB box = Moved(Box(-1, -0.5f, -1.5f, 1, 1, 1.5f), rng.Point(1.5f));
		Contact3D withAabb = Collide(c, box);
		if (withAabb.hit) {
			++tested;
			stuck += Intersects(Moved(c, withAabb.normal * (withAabb.depth + 1e-3f)), box);
		}
	}
	CHECK(tested > 500);
	CHECK(deep > 50); // 軸が食い込むケースも、十分な数を試している
	CHECK(stuck == 0);
}

TEST_CASE(Collision3D_Random_RaycastConsistency) {
	Random rng(2024);
	int mismatches = 0, surfaceErrors = 0, hits = 0;
	for (int n = 0; n < 600; ++n) {
		Segment3D seg{ rng.Point(6.0f), rng.Point(6.0f) };
		Sphere s = Sph(rng.Range(-2, 2), rng.Range(-2, 2), rng.Range(-2, 2), rng.Range(0.3f, 1.5f));
		Capsule c = Cap(rng.Point(2.0f), rng.Point(2.0f), rng.Range(0.3f, 1.0f));
		OBB o = RandomOBB(rng, 2.0f);
		AABB b = Moved(Box(-1, -0.5f, -1.5f, 1, 1, 1.5f), rng.Point(2.0f));

		// 線分の判定（Intersects）と、レイキャストの結果は一致する
		RayHit3D hs = Raycast(seg, s), hc = Raycast(seg, c), ho = Raycast(seg, o), hb = Raycast(seg, b);
		mismatches += hs.hit != Intersects(seg, s);
		mismatches += hc.hit != Intersects(seg, c);
		mismatches += ho.hit != Intersects(seg, o);
		mismatches += hb.hit != Intersects(seg, b);

		// 当たった点は、形の表面上にある（始点が内側のときは除く）
		if (hs.hit && hs.t > 0.0f) { ++hits; surfaceErrors += std::abs(Length(Vector3{ hs.point - s.center }) - s.radius) > 1e-3f; }
		if (hc.hit && hc.t > 0.0f) {
			++hits;
			Vector3 axisPoint = ClosestPoint(hc.point, c.segment);
			surfaceErrors += std::abs(Length(Vector3{ hc.point - axisPoint }) - c.radius) > 1e-3f;
			surfaceErrors += Dot(hc.normal, Vector3{ hc.point - axisPoint }) < 0.0f; // 法線は外向き
		}
		if (ho.hit && ho.t > 0.0f) { ++hits; surfaceErrors += !OnSurface(o, ho.point); }
		if (hb.hit && hb.t > 0.0f) { ++hits; surfaceErrors += !OnSurface(ToOBB(b), hb.point); }

		// 当たった位置より手前（t を少し戻した点）は、形の外側にある
		if (hs.hit && hs.t > 1e-3f) surfaceErrors += Contains(s, seg.start + (seg.end - seg.start) * (hs.t - 1e-3f));
		if (hc.hit && hc.t > 1e-3f) surfaceErrors += Contains(c, seg.start + (seg.end - seg.start) * (hc.t - 1e-3f));

		// カプセルは、線分と軸の最短距離でも判定できる（別の計算との照合）
		Vector3 pa, pb;
		bool byDistance = ClosestPoints(seg, c.segment, &pa, &pb) <= c.radius * c.radius;
		mismatches += byDistance != hc.hit;

		// レイ版（t は距離）は、線分版と同じ点を返す
		Vector3 direction = seg.end - seg.start;
		if (Length(direction) > 1e-3f) {
			Ray ray = MakeRay(seg.start, direction);
			RayHit3D rs = Raycast(ray, s);
			if (rs.hit && hs.hit) surfaceErrors += !Near(rs.point, hs.point, 1e-2f) && hs.t > 0.0f && hs.t < 1.0f;
		}
	}
	CHECK(hits > 100);
	CHECK(mismatches == 0);
	CHECK(surfaceErrors == 0);
}

// ---- 照準（カメラのレイと地面） ----

TEST_CASE(Collision3D_AimingRayHitsGroundAndEnemy) {
	// 斜め上から見下ろすカメラの視線で、地面より手前にいる敵に当たる（照準が敵を選べる）
	Plane ground = MakePlane(V(0, 0, 0), V(0, 1, 0));
	Capsule enemy = MakeUprightCapsule(V(0, 0, 0), 2.0f, 0.5f);
	Vector3 camera = V(0, 8, -8);
	Ray atChest = MakeRay(camera, V(0, 1.0f, 0) - camera); // 敵の胸の高さを通る
	RayHit3D e = Raycast(atChest, enemy);
	RayHit3D g = Raycast(atChest, ground);
	CHECK(e.hit);
	CHECK(g.hit);
	CHECK(e.t < g.t); // 敵のほうが手前
	// 敵の横を通る視線は、敵に当たらず、地面に当たる
	Ray besideEnemy = MakeRay(camera, V(3.0f, 0.0f, 0.0f) - camera);
	CHECK(!Raycast(besideEnemy, enemy).hit);
	CHECK(Raycast(besideEnemy, ground).hit);
}

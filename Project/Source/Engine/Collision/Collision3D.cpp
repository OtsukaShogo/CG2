#include "Collision3D.h"
#include "../Math/Matrix4x4.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace Engine {

namespace {

constexpr float kTiny = 1e-12f;
constexpr float kInfinity = (std::numeric_limits<float>::infinity)();

const Vector3 kZero = { 0.0f, 0.0f, 0.0f };

Vector3 NormalizeOr(const Vector3& v, const Vector3& fallback) {
	float lengthSquared = Dot(v, v);
	return lengthSquared > kTiny ? v / std::sqrt(lengthSquared) : fallback;
}

float AxisOf(const Vector3& v, int axis) { return axis == 0 ? v.x : (axis == 1 ? v.y : v.z); }

Vector3 UnitAxis(int axis, float sign) {
	return Vector3{ axis == 0 ? sign : 0.0f, axis == 1 ? sign : 0.0f, axis == 2 ? sign : 0.0f };
}

// ---- レイキャストの共通部分 ----
// 始点 origin から、方向 direction へ、t=0〜tMax の範囲で進める（線分の場合は direction=終点-始点、tMax=1）

RayHit3D MakeHit(const Vector3& origin, const Vector3& direction, float t, const Vector3& normal) {
	RayHit3D hit;
	hit.hit = true;
	hit.t = t;
	hit.point = origin + direction * t;
	hit.normal = normal;
	return hit;
}

RayHit3D RaycastSphereCore(const Vector3& origin, const Vector3& direction, float tMax, const Sphere& sphere) {
	const Vector3 offset = origin - sphere.center;
	const float c = Dot(offset, offset) - sphere.radius * sphere.radius;
	if (c <= 0.0f) {
		return MakeHit(origin, direction, 0.0f, kZero); // 始点がすでに内側（または表面上）
	}
	const float a = Dot(direction, direction);
	if (a == 0.0f) {
		return RayHit3D{}; // 動かない点で、球の外側
	}
	const float b = Dot(offset, direction);
	const float discriminant = b * b - a * c;
	if (discriminant < 0.0f) {
		return RayHit3D{};
	}
	const float t = (-b - std::sqrt(discriminant)) / a; // 外側から入る側の解
	if (t < 0.0f || t > tMax) {
		return RayHit3D{};
	}
	const Vector3 point = origin + direction * t;
	return MakeHit(origin, direction, t, NormalizeOr(point - sphere.center, kZero));
}

// スラブ法（軸に平行な箱 [lo, hi] との交差）。origin・direction は箱のローカル座標
RayHit3D RaycastSlabs(const float origin[3], const float direction[3], const float lo[3], const float hi[3], float tMax,
	int* outAxis, float* outSign, float* outT) {
	float tEnter = 0.0f;
	float tExit = tMax;
	int enterAxis = -1;
	float enterSign = 0.0f;
	for (int axis = 0; axis < 3; ++axis) {
		if (direction[axis] == 0.0f) {
			// この軸方向には動かない: 範囲外ならどの時刻でも当たらない
			if (origin[axis] < lo[axis] || origin[axis] > hi[axis]) {
				return RayHit3D{};
			}
			continue;
		}
		float t1 = (lo[axis] - origin[axis]) / direction[axis];
		float t2 = (hi[axis] - origin[axis]) / direction[axis];
		float sign = -1.0f; // 下限の面から入る場合、法線は -軸方向
		if (t1 > t2) {
			std::swap(t1, t2);
			sign = 1.0f;
		}
		if (t1 > tEnter) {
			tEnter = t1;
			enterAxis = axis;
			enterSign = sign;
		}
		tExit = (std::min)(tExit, t2);
		if (tEnter > tExit) {
			return RayHit3D{};
		}
	}
	RayHit3D hit;
	hit.hit = true;
	*outAxis = enterAxis;
	*outSign = enterSign;
	*outT = tEnter;
	return hit;
}

RayHit3D RaycastAABBCore(const Vector3& origin, const Vector3& direction, float tMax, const AABB& aabb) {
	const float o[3] = { origin.x, origin.y, origin.z };
	const float d[3] = { direction.x, direction.y, direction.z };
	const float lo[3] = { aabb.min.x, aabb.min.y, aabb.min.z };
	const float hi[3] = { aabb.max.x, aabb.max.y, aabb.max.z };
	int axis = -1;
	float sign = 0.0f, t = 0.0f;
	RayHit3D slab = RaycastSlabs(o, d, lo, hi, tMax, &axis, &sign, &t);
	if (!slab.hit) {
		return slab;
	}
	return MakeHit(origin, direction, t, axis >= 0 ? UnitAxis(axis, sign) : kZero);
}

RayHit3D RaycastOBBCore(const Vector3& origin, const Vector3& direction, float tMax, const OBB& obb) {
	// 箱のローカル座標へ（軸は正規直交なので、内積で変換できる）
	const Vector3 offset = origin - obb.center;
	float o[3], d[3], lo[3], hi[3];
	const float half[3] = { obb.halfExtents.x, obb.halfExtents.y, obb.halfExtents.z };
	for (int i = 0; i < 3; ++i) {
		o[i] = Dot(offset, obb.axes[i]);
		d[i] = Dot(direction, obb.axes[i]);
		lo[i] = -half[i];
		hi[i] = half[i];
	}
	int axis = -1;
	float sign = 0.0f, t = 0.0f;
	RayHit3D slab = RaycastSlabs(o, d, lo, hi, tMax, &axis, &sign, &t);
	if (!slab.hit) {
		return slab;
	}
	return MakeHit(origin, direction, t, axis >= 0 ? obb.axes[axis] * sign : kZero);
}

RayHit3D RaycastCapsuleCore(const Vector3& origin, const Vector3& direction, float tMax, const Capsule& capsule) {
	const Segment3D& segment = capsule.segment;
	const float radius = capsule.radius;

	// 始点がすでにカプセルの内側
	if (Contains(capsule, origin)) {
		return MakeHit(origin, direction, 0.0f, kZero);
	}

	const Vector3 axis = segment.end - segment.start;
	const float length = Length(axis);
	if (length * length <= kTiny) {
		return RaycastSphereCore(origin, direction, tMax, Sphere{ segment.start, radius }); // 長さ0は球
	}
	const Vector3 u = axis / length;

	RayHit3D best;
	best.t = kInfinity;

	// 側面（円柱）: 軸に垂直な成分だけで、円との交点を解く
	const Vector3 m = origin - segment.start;
	const Vector3 mp = m - u * Dot(m, u);
	const Vector3 dp = direction - u * Dot(direction, u);
	const float a = Dot(dp, dp);
	if (a > kTiny) {
		const float b = Dot(mp, dp);
		const float c = Dot(mp, mp) - radius * radius;
		const float discriminant = b * b - a * c;
		if (discriminant >= 0.0f) {
			const float t = (-b - std::sqrt(discriminant)) / a;
			if (t >= 0.0f && t <= tMax) {
				const Vector3 point = origin + direction * t;
				const float s = Dot(point - segment.start, u);
				if (s >= 0.0f && s <= length) { // 両端の半球は、下の球の判定が受け持つ
					best = MakeHit(origin, direction, t, NormalizeOr(point - (segment.start + u * s), kZero));
				}
			}
		}
	}

	// 両端の半球
	for (const Vector3& end : { segment.start, segment.end }) {
		RayHit3D cap = RaycastSphereCore(origin, direction, tMax, Sphere{ end, radius });
		if (cap.hit && cap.t < best.t) {
			best = cap;
		}
	}

	if (best.t == kInfinity) {
		return RayHit3D{};
	}
	return best;
}

RayHit3D RaycastPlaneCore(const Vector3& origin, const Vector3& direction, float tMax, const Plane& plane) {
	const float originDistance = Dot(plane.normal, origin) - plane.distance;
	if (originDistance == 0.0f) {
		return MakeHit(origin, direction, 0.0f, plane.normal); // 始点が平面上
	}
	const float denominator = Dot(plane.normal, direction);
	if (std::abs(denominator) < kTiny) {
		return RayHit3D{}; // 平面と平行
	}
	const float t = -originDistance / denominator;
	if (t < 0.0f || t > tMax) {
		return RayHit3D{};
	}
	// 法線は、始点がいる側（レイが来る側）を向ける
	return MakeHit(origin, direction, t, originDistance > 0.0f ? plane.normal : -plane.normal);
}

// ---- 分離軸（SAT）による、箱どうしの判定 ----
// contact が nullptr なら、当たっているかだけを返す。normal は a を b から離す向き
bool SeparatingAxisTest(const OBB& a, const OBB& b, Contact3D* contact) {
	const Vector3 t = b.center - a.center;
	const float ha[3] = { a.halfExtents.x, a.halfExtents.y, a.halfExtents.z };
	const float hb[3] = { b.halfExtents.x, b.halfExtents.y, b.halfExtents.z };

	float bestOverlap = kInfinity;
	Vector3 bestNormal = kZero;

	// 軸 L（長さ1）について、投影が重なっているかを調べる。分離していれば false
	auto testAxis = [&](const Vector3& axis, bool isFaceAxis) {
		float ra = 0.0f, rb = 0.0f;
		for (int i = 0; i < 3; ++i) {
			ra += ha[i] * std::abs(Dot(a.axes[i], axis));
			rb += hb[i] * std::abs(Dot(b.axes[i], axis));
		}
		const float distance = Dot(t, axis);
		const float overlap = ra + rb - std::abs(distance);
		if (overlap < 0.0f) {
			return false;
		}
		// 辺どうしの軸は、面の軸とほぼ同じ深さなら選ばない（面から押し出すほうが、安定するため）
		const float bias = isFaceAxis ? 0.0f : 1e-4f;
		if (overlap + bias < bestOverlap) {
			bestOverlap = overlap;
			bestNormal = distance > 0.0f ? -axis : axis; // bが+側にいるなら、aは-側へ
		}
		return true;
	};

	for (int i = 0; i < 3; ++i) {
		if (!testAxis(a.axes[i], true)) return false;
	}
	for (int i = 0; i < 3; ++i) {
		if (!testAxis(b.axes[i], true)) return false;
	}
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			const Vector3 c = Cross(a.axes[i], b.axes[j]);
			const float lengthSquared = Dot(c, c);
			if (lengthSquared < 1e-6f) {
				continue; // 2つの軸がほぼ平行: 面の軸でカバーされるので、飛ばす
			}
			if (!testAxis(c / std::sqrt(lengthSquared), false)) return false;
		}
	}

	if (contact) {
		contact->hit = true;
		contact->normal = bestNormal;
		contact->depth = bestOverlap;
	}
	return true;
}

// 球（中心・半径）と箱の衝突
Contact3D SphereVsOBB(const Vector3& center, float radius, const OBB& obb) {
	Contact3D contact;
	const Vector3 offset = center - obb.center;
	const float half[3] = { obb.halfExtents.x, obb.halfExtents.y, obb.halfExtents.z };
	float local[3];
	bool inside = true;
	for (int i = 0; i < 3; ++i) {
		local[i] = Dot(offset, obb.axes[i]);
		inside = inside && std::abs(local[i]) <= half[i];
	}

	if (!inside) {
		// 中心が箱の外側: 最も近い点から、中心へ押し出す
		const Vector3 delta = center - ClosestPoint(center, obb);
		const float distanceSquared = Dot(delta, delta);
		if (distanceSquared > radius * radius) {
			return contact;
		}
		const float distance = std::sqrt(distanceSquared);
		contact.hit = true;
		contact.normal = delta / distance;
		contact.depth = radius - distance;
		return contact;
	}

	// 中心が箱の内側（または表面上）: 最も近い面から、外へ押し出す。
	// 「内側か」は、最近接点との距離が0かではなく、ローカル座標で直接調べる
	// （最近接点は、軸ごとの足し合わせで求めるので、内側でも、浮動小数点の誤差でわずかにずれるため）
	contact.hit = true;
	float nearest = kInfinity;
	for (int i = 0; i < 3; ++i) {
		const float toFace = half[i] - std::abs(local[i]);
		if (toFace < nearest) {
			nearest = toFace;
			contact.normal = obb.axes[i] * (local[i] >= 0.0f ? 1.0f : -1.0f);
		}
	}
	contact.depth = nearest + radius;
	return contact;
}

// 線分とOBBの最近接点（交差していれば距離0）。距離の2乗を返す
float SegmentOBBDistanceSquared(const Segment3D& segment, const OBB& obb, Vector3* outSegment, Vector3* outBox) {
	// 交差していれば距離0
	RayHit3D hit = Raycast(segment, obb);
	if (hit.hit) {
		*outSegment = hit.point;
		*outBox = hit.point;
		return 0.0f;
	}

	// 交差していなければ、最近接ペアは、「線分の端点と箱」か「線分と箱の12本の辺」のどちらか
	float best = kInfinity;
	auto consider = [&](const Vector3& onSegment, const Vector3& onBox) {
		const Vector3 d = onSegment - onBox;
		const float distanceSquared = Dot(d, d);
		if (distanceSquared < best) {
			best = distanceSquared;
			*outSegment = onSegment;
			*outBox = onBox;
		}
	};
	for (const Vector3& end : { segment.start, segment.end }) {
		consider(end, ClosestPoint(end, obb));
	}
	Vector3 corners[8];
	GetCorners(obb, corners);
	for (int i = 0; i < 8; ++i) {
		for (int bit = 0; bit < 3; ++bit) {
			if (i & (1 << bit)) {
				continue; // 各辺は、該当ビットが0の頂点から1回だけ数える
			}
			Vector3 onSegment, onEdge;
			(void)ClosestPoints(segment, Segment3D{ corners[i], corners[i | (1 << bit)] }, &onSegment, &onEdge);
			consider(onSegment, onEdge);
		}
	}
	return best;
}

Contact3D CapsuleVsOBB(const Capsule& capsule, const OBB& obb) {
	Contact3D contact;
	Vector3 onSegment, onBox;
	const float distanceSquared = SegmentOBBDistanceSquared(capsule.segment, obb, &onSegment, &onBox);
	if (distanceSquared > capsule.radius * capsule.radius) {
		return contact;
	}
	if (distanceSquared > 0.0f) {
		const float distance = std::sqrt(distanceSquared);
		contact.hit = true;
		contact.normal = (onSegment - onBox) / distance;
		contact.depth = capsule.radius - distance;
		return contact;
	}

	// 軸が箱に食い込んでいる: 分離軸のうち、重なりが最も浅い軸の向きへ押し出す。
	// 軸は、箱の3つの面の法線・カプセルの軸・カプセルの軸と箱の辺の外積。どの軸も、その重なり分だけ動かせば、必ず離れる
	const Vector3 capsuleCenter = (capsule.segment.start + capsule.segment.end) * 0.5f;
	const Vector3 halfAxis = (capsule.segment.end - capsule.segment.start) * 0.5f;
	const Vector3 toCapsule = capsuleCenter - obb.center;
	const float half[3] = { obb.halfExtents.x, obb.halfExtents.y, obb.halfExtents.z };

	float bestOverlap = kInfinity;
	Vector3 bestNormal = obb.axes[0];
	auto testAxis = [&](const Vector3& axis) {
		float boxExtent = 0.0f;
		for (int i = 0; i < 3; ++i) {
			boxExtent += half[i] * std::abs(Dot(obb.axes[i], axis));
		}
		const float capsuleExtent = std::abs(Dot(halfAxis, axis)) + capsule.radius;
		const float distance = Dot(toCapsule, axis);
		const float overlap = boxExtent + capsuleExtent - std::abs(distance);
		if (overlap < bestOverlap) {
			bestOverlap = overlap;
			bestNormal = distance >= 0.0f ? axis : -axis; // カプセルのいる側へ
		}
	};
	for (int i = 0; i < 3; ++i) {
		testAxis(obb.axes[i]);
	}
	const Vector3 capsuleDirection = NormalizeOr(halfAxis, kZero);
	if (capsuleDirection != kZero) {
		testAxis(capsuleDirection);
		for (int i = 0; i < 3; ++i) {
			const Vector3 cross = Cross(obb.axes[i], capsuleDirection);
			if (Dot(cross, cross) > 1e-6f) {
				testAxis(cross / std::sqrt(Dot(cross, cross)));
			}
		}
	}

	contact.hit = true;
	contact.normal = bestNormal;
	contact.depth = bestOverlap;
	return contact;
}

} // namespace

// ---- 形の作成 ----

OBB MakeOBB(const Vector3& center, const Vector3& halfExtents, const Quaternion& rotation) {
	const Matrix4x4 m = MakeRotateMatrix(rotation);
	OBB obb;
	obb.center = center;
	obb.halfExtents = halfExtents;
	for (int i = 0; i < 3; ++i) {
		obb.axes[i] = Vector3{ m.m[i][0], m.m[i][1], m.m[i][2] }; // 行ベクトル規約: 行iが、ローカルのi軸の行き先
	}
	return obb;
}

OBB MakeOBB(const Vector3& center, const Vector3& halfExtents, const Vector3& eulerRadians) {
	return MakeOBB(center, halfExtents, MakeQuaternionFromEuler(eulerRadians));
}

OBB ToOBB(const AABB& aabb) {
	OBB obb;
	obb.center = (aabb.min + aabb.max) * 0.5f;
	obb.halfExtents = (aabb.max - aabb.min) * 0.5f;
	return obb;
}

Capsule MakeUprightCapsule(const Vector3& foot, float height, float radius) {
	Capsule capsule;
	capsule.radius = radius;
	if (height <= radius * 2.0f) {
		const Vector3 middle = foot + Vector3{ 0.0f, height * 0.5f, 0.0f };
		capsule.segment = Segment3D{ middle, middle };
	} else {
		capsule.segment = Segment3D{ foot + Vector3{ 0.0f, radius, 0.0f }, foot + Vector3{ 0.0f, height - radius, 0.0f } };
	}
	return capsule;
}

Plane MakePlane(const Vector3& point, const Vector3& normal) {
	Plane plane;
	plane.normal = NormalizeOr(normal, Vector3{ 0.0f, 1.0f, 0.0f });
	plane.distance = Dot(plane.normal, point);
	return plane;
}

void GetCorners(const OBB& obb, Vector3 outCorners[8]) {
	for (int i = 0; i < 8; ++i) {
		outCorners[i] = obb.center
			+ obb.axes[0] * ((i & 1) ? obb.halfExtents.x : -obb.halfExtents.x)
			+ obb.axes[1] * ((i & 2) ? obb.halfExtents.y : -obb.halfExtents.y)
			+ obb.axes[2] * ((i & 4) ? obb.halfExtents.z : -obb.halfExtents.z);
	}
}

// ---- 最近接点・距離 ----

Vector3 ClosestPoint(const Vector3& point, const AABB& aabb) {
	return Vector3{ (std::clamp)(point.x, aabb.min.x, aabb.max.x), (std::clamp)(point.y, aabb.min.y, aabb.max.y), (std::clamp)(point.z, aabb.min.z, aabb.max.z) };
}

Vector3 ClosestPoint(const Vector3& point, const OBB& obb) {
	const Vector3 offset = point - obb.center;
	const float half[3] = { obb.halfExtents.x, obb.halfExtents.y, obb.halfExtents.z };
	Vector3 result = obb.center;
	for (int i = 0; i < 3; ++i) {
		result += obb.axes[i] * (std::clamp)(Dot(offset, obb.axes[i]), -half[i], half[i]);
	}
	return result;
}

Vector3 ClosestPoint(const Vector3& point, const Segment3D& segment) {
	const Vector3 direction = segment.end - segment.start;
	const float lengthSquared = Dot(direction, direction);
	if (lengthSquared == 0.0f) {
		return segment.start; // 長さ0の線分（0除算を避ける）
	}
	const float t = (std::clamp)(Dot(point - segment.start, direction) / lengthSquared, 0.0f, 1.0f);
	return segment.start + direction * t;
}

float SignedDistance(const Plane& plane, const Vector3& point) {
	return Dot(plane.normal, point) - plane.distance;
}

float ClosestPoints(const Segment3D& a, const Segment3D& b, Vector3* outA, Vector3* outB) {
	const Vector3 d1 = a.end - a.start;
	const Vector3 d2 = b.end - b.start;
	const Vector3 r = a.start - b.start;
	const float aa = Dot(d1, d1);
	const float e = Dot(d2, d2);
	const float f = Dot(d2, r);
	float s = 0.0f, t = 0.0f;

	if (aa <= kTiny && e <= kTiny) {
		// どちらも点
	} else if (aa <= kTiny) {
		t = (std::clamp)(f / e, 0.0f, 1.0f);
	} else {
		const float c = Dot(d1, r);
		if (e <= kTiny) {
			s = (std::clamp)(-c / aa, 0.0f, 1.0f);
		} else {
			const float bb = Dot(d1, d2);
			const float denominator = aa * e - bb * bb; // 常に0以上。0に近いほど平行
			if (denominator > 1e-9f * aa * e) {
				s = (std::clamp)((bb * f - c * e) / denominator, 0.0f, 1.0f);
			}
			// s が決まったので、b 上の最近点 t を求める。範囲外なら端に丸めて、s を求め直す
			const float tNumerator = bb * s + f;
			if (tNumerator < 0.0f) {
				t = 0.0f;
				s = (std::clamp)(-c / aa, 0.0f, 1.0f);
			} else if (tNumerator > e) {
				t = 1.0f;
				s = (std::clamp)((bb - c) / aa, 0.0f, 1.0f);
			} else {
				t = tNumerator / e;
			}
		}
	}

	*outA = a.start + d1 * s;
	*outB = b.start + d2 * t;
	const Vector3 d = *outA - *outB;
	return Dot(d, d);
}

float ClosestPoints(const Segment3D& segment, const OBB& obb, Vector3* outSegment, Vector3* outBox) {
	return SegmentOBBDistanceSquared(segment, obb, outSegment, outBox);
}

float ClosestPoints(const Segment3D& segment, const AABB& aabb, Vector3* outSegment, Vector3* outBox) {
	return SegmentOBBDistanceSquared(segment, ToOBB(aabb), outSegment, outBox);
}

bool Contains(const Sphere& sphere, const Vector3& point) {
	const Vector3 d = point - sphere.center;
	return Dot(d, d) <= sphere.radius * sphere.radius;
}

bool Contains(const OBB& obb, const Vector3& point) {
	const Vector3 offset = point - obb.center;
	return std::abs(Dot(offset, obb.axes[0])) <= obb.halfExtents.x
		&& std::abs(Dot(offset, obb.axes[1])) <= obb.halfExtents.y
		&& std::abs(Dot(offset, obb.axes[2])) <= obb.halfExtents.z;
}

bool Contains(const Capsule& capsule, const Vector3& point) {
	const Vector3 d = point - ClosestPoint(point, capsule.segment);
	return Dot(d, d) <= capsule.radius * capsule.radius;
}

// ---- 判定のみ ----

bool Intersects(const Sphere& a, const Sphere& b) {
	const Vector3 d = b.center - a.center;
	const float radiusSum = a.radius + b.radius;
	return Dot(d, d) <= radiusSum * radiusSum;
}

bool Intersects(const Sphere& sphere, const AABB& aabb) {
	const Vector3 d = sphere.center - ClosestPoint(sphere.center, aabb);
	return Dot(d, d) <= sphere.radius * sphere.radius;
}

bool Intersects(const Sphere& sphere, const OBB& obb) {
	const Vector3 d = sphere.center - ClosestPoint(sphere.center, obb);
	return Dot(d, d) <= sphere.radius * sphere.radius;
}

bool Intersects(const Sphere& sphere, const Capsule& capsule) {
	const Vector3 d = sphere.center - ClosestPoint(sphere.center, capsule.segment);
	const float radiusSum = sphere.radius + capsule.radius;
	return Dot(d, d) <= radiusSum * radiusSum;
}

bool Intersects(const Sphere& sphere, const Plane& plane) {
	return std::abs(SignedDistance(plane, sphere.center)) <= sphere.radius;
}

bool Intersects(const AABB& a, const AABB& b) {
	return a.min.x <= b.max.x && a.max.x >= b.min.x
		&& a.min.y <= b.max.y && a.max.y >= b.min.y
		&& a.min.z <= b.max.z && a.max.z >= b.min.z;
}

bool Intersects(const AABB& aabb, const OBB& obb) {
	return SeparatingAxisTest(ToOBB(aabb), obb, nullptr);
}

bool Intersects(const OBB& a, const OBB& b) {
	return SeparatingAxisTest(a, b, nullptr);
}

bool Intersects(const Capsule& a, const Capsule& b) {
	Vector3 pa, pb;
	const float radiusSum = a.radius + b.radius;
	return ClosestPoints(a.segment, b.segment, &pa, &pb) <= radiusSum * radiusSum;
}

bool Intersects(const Capsule& capsule, const AABB& aabb) {
	Vector3 onSegment, onBox;
	return ClosestPoints(capsule.segment, aabb, &onSegment, &onBox) <= capsule.radius * capsule.radius;
}

bool Intersects(const Capsule& capsule, const OBB& obb) {
	Vector3 onSegment, onBox;
	return ClosestPoints(capsule.segment, obb, &onSegment, &onBox) <= capsule.radius * capsule.radius;
}

bool Intersects(const Capsule& capsule, const Plane& plane) {
	const float distanceStart = SignedDistance(plane, capsule.segment.start);
	const float distanceEnd = SignedDistance(plane, capsule.segment.end);
	if (distanceStart * distanceEnd <= 0.0f) {
		return true; // 軸が平面を跨いでいる
	}
	return (std::min)(std::abs(distanceStart), std::abs(distanceEnd)) <= capsule.radius;
}

bool Intersects(const AABB& aabb, const Plane& plane) {
	const Vector3 center = (aabb.min + aabb.max) * 0.5f;
	const Vector3 half = (aabb.max - aabb.min) * 0.5f;
	const float projected = half.x * std::abs(plane.normal.x) + half.y * std::abs(plane.normal.y) + half.z * std::abs(plane.normal.z);
	return std::abs(SignedDistance(plane, center)) <= projected;
}

bool Intersects(const OBB& obb, const Plane& plane) {
	float projected = 0.0f;
	const float half[3] = { obb.halfExtents.x, obb.halfExtents.y, obb.halfExtents.z };
	for (int i = 0; i < 3; ++i) {
		projected += half[i] * std::abs(Dot(plane.normal, obb.axes[i]));
	}
	return std::abs(SignedDistance(plane, obb.center)) <= projected;
}

bool Intersects(const Segment3D& segment, const Sphere& sphere) {
	const Vector3 d = sphere.center - ClosestPoint(sphere.center, segment);
	return Dot(d, d) <= sphere.radius * sphere.radius;
}

bool Intersects(const Segment3D& segment, const AABB& aabb) {
	return Raycast(segment, aabb).hit;
}

bool Intersects(const Segment3D& segment, const OBB& obb) {
	return Raycast(segment, obb).hit;
}

bool Intersects(const Segment3D& segment, const Capsule& capsule) {
	Vector3 pa, pb;
	return ClosestPoints(segment, capsule.segment, &pa, &pb) <= capsule.radius * capsule.radius;
}

bool Intersects(const Segment3D& segment, const Plane& plane) {
	return SignedDistance(plane, segment.start) * SignedDistance(plane, segment.end) <= 0.0f;
}

// ---- 衝突の詳細 ----

Contact3D FlipContact(Contact3D contact) {
	contact.normal = -contact.normal;
	return contact;
}

Contact3D Collide(const Sphere& a, const Sphere& b) {
	Contact3D contact;
	const Vector3 delta = a.center - b.center; // bからaへ
	const float radiusSum = a.radius + b.radius;
	const float distanceSquared = Dot(delta, delta);
	if (distanceSquared > radiusSum * radiusSum) {
		return contact;
	}
	contact.hit = true;
	const float distance = std::sqrt(distanceSquared);
	contact.normal = distance > 1e-6f ? delta / distance : Vector3{ 0.0f, 1.0f, 0.0f }; // 中心が一致していて向きが決まらない
	contact.depth = radiusSum - distance;
	return contact;
}

Contact3D Collide(const Sphere& sphere, const AABB& aabb) {
	return SphereVsOBB(sphere.center, sphere.radius, ToOBB(aabb));
}

Contact3D Collide(const Sphere& sphere, const OBB& obb) {
	return SphereVsOBB(sphere.center, sphere.radius, obb);
}

Contact3D Collide(const Sphere& sphere, const Capsule& capsule) {
	return Collide(sphere, Sphere{ ClosestPoint(sphere.center, capsule.segment), capsule.radius });
}

Contact3D Collide(const Sphere& sphere, const Plane& plane) {
	Contact3D contact;
	const float distance = SignedDistance(plane, sphere.center);
	if (std::abs(distance) > sphere.radius) {
		return contact;
	}
	contact.hit = true;
	contact.normal = distance >= 0.0f ? plane.normal : -plane.normal;
	contact.depth = sphere.radius - std::abs(distance);
	return contact;
}

Contact3D Collide(const AABB& a, const AABB& b) {
	Contact3D contact;
	const float overlap[3] = {
		(std::min)(a.max.x, b.max.x) - (std::max)(a.min.x, b.min.x),
		(std::min)(a.max.y, b.max.y) - (std::max)(a.min.y, b.min.y),
		(std::min)(a.max.z, b.max.z) - (std::max)(a.min.z, b.min.z),
	};
	if (overlap[0] < 0.0f || overlap[1] < 0.0f || overlap[2] < 0.0f) {
		return contact;
	}
	int axis = 0;
	for (int i = 1; i < 3; ++i) {
		if (overlap[i] < overlap[axis]) {
			axis = i;
		}
	}
	const Vector3 centerA = (a.min + a.max) * 0.5f;
	const Vector3 centerB = (b.min + b.max) * 0.5f;
	contact.hit = true;
	contact.depth = overlap[axis];
	contact.normal = UnitAxis(axis, AxisOf(centerA, axis) < AxisOf(centerB, axis) ? -1.0f : 1.0f);
	return contact;
}

Contact3D Collide(const AABB& aabb, const OBB& obb) {
	Contact3D contact;
	SeparatingAxisTest(ToOBB(aabb), obb, &contact);
	return contact;
}

Contact3D Collide(const OBB& a, const OBB& b) {
	Contact3D contact;
	SeparatingAxisTest(a, b, &contact);
	return contact;
}

Contact3D Collide(const Capsule& a, const Capsule& b) {
	Contact3D contact;
	Vector3 pa, pb;
	const float radiusSum = a.radius + b.radius;
	const float distanceSquared = ClosestPoints(a.segment, b.segment, &pa, &pb);
	if (distanceSquared > radiusSum * radiusSum) {
		return contact;
	}
	contact.hit = true;
	if (distanceSquared > 1e-10f) {
		const float distance = std::sqrt(distanceSquared);
		contact.normal = (pa - pb) / distance;
		contact.depth = radiusSum - distance;
		return contact;
	}
	// 2本の軸が交差している（誤差で、距離がごく小さい場合も含む）: 両方の軸に垂直な向き（平行なら、aの軸に垂直な任意の向き）へ押し出す
	const Vector3 da = a.segment.end - a.segment.start;
	const Vector3 db = b.segment.end - b.segment.start;
	Vector3 axis = Cross(da, db);
	if (Dot(axis, axis) <= kTiny) {
		axis = Perpendicular(NormalizeOr(da, Vector3{ 0.0f, 1.0f, 0.0f }));
	}
	contact.normal = NormalizeOr(axis, Vector3{ 0.0f, 1.0f, 0.0f });
	// 向きは、bの中心からaの中心へ向く側にそろえる
	const Vector3 centers = (a.segment.start + a.segment.end) * 0.5f - (b.segment.start + b.segment.end) * 0.5f;
	if (Dot(contact.normal, centers) < 0.0f) {
		contact.normal = -contact.normal;
	}
	contact.depth = radiusSum;
	return contact;
}

Contact3D Collide(const Capsule& capsule, const AABB& aabb) {
	return CapsuleVsOBB(capsule, ToOBB(aabb));
}

Contact3D Collide(const Capsule& capsule, const OBB& obb) {
	return CapsuleVsOBB(capsule, obb);
}

Contact3D Collide(const Capsule& capsule, const Plane& plane) {
	Contact3D contact;
	const float distanceStart = SignedDistance(plane, capsule.segment.start);
	const float distanceEnd = SignedDistance(plane, capsule.segment.end);
	// 軸の中点がある側へ押し出す。その側から見た、軸の両端の高さのうち低いほうが、最も深い点
	const float side = (distanceStart + distanceEnd) >= 0.0f ? 1.0f : -1.0f;
	const float lowest = (std::min)(side * distanceStart, side * distanceEnd); // 跨いでいる場合は負になる
	if (lowest > capsule.radius) {
		return contact;
	}
	contact.hit = true;
	contact.normal = plane.normal * side;
	contact.depth = capsule.radius - lowest;
	return contact;
}

// ---- レイキャスト ----

RayHit3D Raycast(const Segment3D& segment, const Sphere& sphere) {
	return RaycastSphereCore(segment.start, segment.end - segment.start, 1.0f, sphere);
}

RayHit3D Raycast(const Segment3D& segment, const AABB& aabb) {
	return RaycastAABBCore(segment.start, segment.end - segment.start, 1.0f, aabb);
}

RayHit3D Raycast(const Segment3D& segment, const OBB& obb) {
	return RaycastOBBCore(segment.start, segment.end - segment.start, 1.0f, obb);
}

RayHit3D Raycast(const Segment3D& segment, const Capsule& capsule) {
	return RaycastCapsuleCore(segment.start, segment.end - segment.start, 1.0f, capsule);
}

RayHit3D Raycast(const Segment3D& segment, const Plane& plane) {
	return RaycastPlaneCore(segment.start, segment.end - segment.start, 1.0f, plane);
}

RayHit3D Raycast(const Ray& ray, const Sphere& sphere, float maxDistance) {
	return RaycastSphereCore(ray.origin, ray.direction, maxDistance, sphere);
}

RayHit3D Raycast(const Ray& ray, const AABB& aabb, float maxDistance) {
	return RaycastAABBCore(ray.origin, ray.direction, maxDistance, aabb);
}

RayHit3D Raycast(const Ray& ray, const OBB& obb, float maxDistance) {
	return RaycastOBBCore(ray.origin, ray.direction, maxDistance, obb);
}

RayHit3D Raycast(const Ray& ray, const Capsule& capsule, float maxDistance) {
	return RaycastCapsuleCore(ray.origin, ray.direction, maxDistance, capsule);
}

RayHit3D Raycast(const Ray& ray, const Plane& plane, float maxDistance) {
	return RaycastPlaneCore(ray.origin, ray.direction, maxDistance, plane);
}

} // namespace Engine

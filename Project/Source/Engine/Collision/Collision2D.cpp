#include "Collision2D.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace Engine {

namespace {

float Dot2(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }
float LengthSquared2(const Vector2& v) { return Dot2(v, v); }

float Cross2(const Vector2& a, const Vector2& b) { return a.x * b.y - a.y * b.x; }

// 値の符号（-1, 0, 1）
int Sign(float v) { return (v > 0.0f) - (v < 0.0f); }

// 点cが、線分ab上（aとbの外接矩形の内側）にあるか。a・b・cが同一直線上にあることが前提
bool OnSegmentBounds(const Vector2& a, const Vector2& b, const Vector2& c) {
	return c.x >= (std::min)(a.x, b.x) && c.x <= (std::max)(a.x, b.x) &&
		c.y >= (std::min)(a.y, b.y) && c.y <= (std::max)(a.y, b.y);
}

} // namespace

Vector2 ClosestPoint(const Vector2& point, const AABB2D& aabb) {
	return Vector2{ (std::clamp)(point.x, aabb.min.x, aabb.max.x), (std::clamp)(point.y, aabb.min.y, aabb.max.y) };
}

Vector2 ClosestPoint(const Vector2& point, const Segment2D& segment) {
	Vector2 direction = segment.end - segment.start;
	float lengthSquared = LengthSquared2(direction);
	if (lengthSquared == 0.0f) {
		return segment.start; // 長さ0の線分（0除算を避ける）
	}
	float t = (std::clamp)(Dot2(point - segment.start, direction) / lengthSquared, 0.0f, 1.0f);
	return segment.start + direction * t;
}

bool Contains(const Circle2D& circle, const Vector2& point) {
	return LengthSquared2(point - circle.center) <= circle.radius * circle.radius;
}

bool Contains(const AABB2D& aabb, const Vector2& point) {
	return point.x >= aabb.min.x && point.x <= aabb.max.x && point.y >= aabb.min.y && point.y <= aabb.max.y;
}

bool Intersects(const Circle2D& a, const Circle2D& b) {
	float radiusSum = a.radius + b.radius;
	return LengthSquared2(b.center - a.center) <= radiusSum * radiusSum;
}

bool Intersects(const Circle2D& circle, const AABB2D& aabb) {
	Vector2 closest = ClosestPoint(circle.center, aabb);
	return LengthSquared2(circle.center - closest) <= circle.radius * circle.radius;
}

bool Intersects(const AABB2D& aabb, const Circle2D& circle) {
	return Intersects(circle, aabb);
}

bool Intersects(const AABB2D& a, const AABB2D& b) {
	return a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y && a.max.y >= b.min.y;
}

bool Intersects(const Segment2D& segment, const Circle2D& circle) {
	Vector2 closest = ClosestPoint(circle.center, segment);
	return LengthSquared2(circle.center - closest) <= circle.radius * circle.radius;
}

bool Intersects(const Circle2D& circle, const Segment2D& segment) {
	return Intersects(segment, circle);
}

bool Intersects(const Segment2D& segment, const AABB2D& aabb) {
	return Raycast(segment, aabb).hit;
}

bool Intersects(const AABB2D& aabb, const Segment2D& segment) {
	return Intersects(segment, aabb);
}

bool Intersects(const Segment2D& a, const Segment2D& b) {
	// 各線分の両端が、もう一方の線分の反対側にあれば交差する。同一直線上（向きが0）の場合は、範囲の重なりで判定する
	const Vector2 ab = a.end - a.start;
	const Vector2 cd = b.end - b.start;
	const int d1 = Sign(Cross2(ab, b.start - a.start));
	const int d2 = Sign(Cross2(ab, b.end - a.start));
	const int d3 = Sign(Cross2(cd, a.start - b.start));
	const int d4 = Sign(Cross2(cd, a.end - b.start));

	if (d1 * d2 < 0 && d3 * d4 < 0) {
		return true;
	}
	if (d1 == 0 && OnSegmentBounds(a.start, a.end, b.start)) return true;
	if (d2 == 0 && OnSegmentBounds(a.start, a.end, b.end)) return true;
	if (d3 == 0 && OnSegmentBounds(b.start, b.end, a.start)) return true;
	if (d4 == 0 && OnSegmentBounds(b.start, b.end, a.end)) return true;
	return false;
}

Contact2D Collide(const Circle2D& a, const Circle2D& b) {
	Contact2D contact;
	const Vector2 delta = a.center - b.center; // bからaへ
	const float radiusSum = a.radius + b.radius;
	const float distanceSquared = LengthSquared2(delta);
	if (distanceSquared > radiusSum * radiusSum) {
		return contact;
	}
	contact.hit = true;
	const float distance = std::sqrt(distanceSquared);
	if (distance > 0.0f) {
		contact.normal = delta / distance;
	} else {
		contact.normal = Vector2{ 1.0f, 0.0f }; // 中心が一致していて向きが決まらない
	}
	contact.depth = radiusSum - distance;
	return contact;
}

Contact2D Collide(const Circle2D& circle, const AABB2D& aabb) {
	Contact2D contact;
	const Vector2 closest = ClosestPoint(circle.center, aabb);
	const Vector2 delta = circle.center - closest; // 矩形から円へ
	const float distanceSquared = LengthSquared2(delta);
	if (distanceSquared > circle.radius * circle.radius) {
		return contact;
	}
	contact.hit = true;

	if (distanceSquared > 0.0f) {
		// 中心が矩形の外側: 最も近い点から円の中心へ押し出す
		const float distance = std::sqrt(distanceSquared);
		contact.normal = delta / distance;
		contact.depth = circle.radius - distance;
		return contact;
	}

	// 中心が矩形の内側（または境界上）: 最も近い面から外へ押し出す
	const float toLeft = circle.center.x - aabb.min.x;
	const float toRight = aabb.max.x - circle.center.x;
	const float toBottom = circle.center.y - aabb.min.y;
	const float toTop = aabb.max.y - circle.center.y;
	float nearest = toLeft;
	contact.normal = Vector2{ -1.0f, 0.0f };
	if (toRight < nearest) { nearest = toRight;  contact.normal = Vector2{ 1.0f, 0.0f }; }
	if (toBottom < nearest) { nearest = toBottom; contact.normal = Vector2{ 0.0f, -1.0f }; }
	if (toTop < nearest) { nearest = toTop;    contact.normal = Vector2{ 0.0f, 1.0f }; }
	contact.depth = nearest + circle.radius; // 面から外へ出て、さらに半径ぶん離れる
	return contact;
}

Contact2D Collide(const AABB2D& aabb, const Circle2D& circle) {
	Contact2D contact = Collide(circle, aabb);
	contact.normal = Vector2{ -contact.normal.x, -contact.normal.y }; // 動かす対象が逆になるので、向きも逆
	return contact;
}

Contact2D Collide(const AABB2D& a, const AABB2D& b) {
	Contact2D contact;
	const float overlapX = (std::min)(a.max.x, b.max.x) - (std::max)(a.min.x, b.min.x);
	const float overlapY = (std::min)(a.max.y, b.max.y) - (std::max)(a.min.y, b.min.y);
	if (overlapX < 0.0f || overlapY < 0.0f) {
		return contact;
	}
	contact.hit = true;
	const Vector2 centerA = (a.min + a.max) * 0.5f;
	const Vector2 centerB = (b.min + b.max) * 0.5f;
	if (overlapX <= overlapY) {
		contact.depth = overlapX;
		contact.normal = Vector2{ centerA.x < centerB.x ? -1.0f : 1.0f, 0.0f };
	} else {
		contact.depth = overlapY;
		contact.normal = Vector2{ 0.0f, centerA.y < centerB.y ? -1.0f : 1.0f };
	}
	return contact;
}

RayHit2D Raycast(const Segment2D& segment, const Circle2D& circle) {
	RayHit2D result;
	const Vector2 direction = segment.end - segment.start;
	const Vector2 offset = segment.start - circle.center;
	const float c = LengthSquared2(offset) - circle.radius * circle.radius;

	// 始点がすでに円の内側（または円周上）
	if (c <= 0.0f) {
		result.hit = true;
		result.t = 0.0f;
		result.point = segment.start;
		return result;
	}

	const float a = LengthSquared2(direction);
	if (a == 0.0f) {
		return result; // 動かない点で、円の外側
	}
	const float b = Dot2(offset, direction);          // 2で割った係数（半分の形で解く）
	const float discriminant = b * b - a * c;
	if (discriminant < 0.0f) {
		return result;
	}
	// 外側から入る側の解（小さいほう）。c>0 なので、b<0（円へ向かっている）のときだけ有効
	const float t = (-b - std::sqrt(discriminant)) / a;
	if (t < 0.0f || t > 1.0f) {
		return result;
	}
	result.hit = true;
	result.t = t;
	result.point = segment.start + direction * t;
	Vector2 toPoint = result.point - circle.center;
	float length = std::sqrt(LengthSquared2(toPoint));
	result.normal = length > 0.0f ? toPoint / length : Vector2{ 0.0f, 0.0f };
	return result;
}

RayHit2D Raycast(const Segment2D& segment, const AABB2D& aabb) {
	RayHit2D result;
	const Vector2 direction = segment.end - segment.start;
	const float start[2] = { segment.start.x, segment.start.y };
	const float dir[2] = { direction.x, direction.y };
	const float lo[2] = { aabb.min.x, aabb.min.y };
	const float hi[2] = { aabb.max.x, aabb.max.y };

	// スラブ法: 各軸で、矩形の範囲に入る・出る時刻を求め、全軸で重なる区間があるかを調べる
	float tEnter = 0.0f;
	float tExit = 1.0f;
	int enterAxis = -1;
	float enterSign = 0.0f;
	for (int axis = 0; axis < 2; ++axis) {
		if (dir[axis] == 0.0f) {
			// この軸方向には動かない: 範囲外ならどの時刻でも当たらない
			if (start[axis] < lo[axis] || start[axis] > hi[axis]) {
				return result;
			}
			continue;
		}
		float t1 = (lo[axis] - start[axis]) / dir[axis];
		float t2 = (hi[axis] - start[axis]) / dir[axis];
		float sign = -1.0f; // 下限の面から入る場合、法線は -軸方向
		if (t1 > t2) {
			std::swap(t1, t2);
			sign = 1.0f;    // 上限の面から入る場合、法線は +軸方向
		}
		if (t1 > tEnter) {
			tEnter = t1;
			enterAxis = axis;
			enterSign = sign;
		}
		tExit = (std::min)(tExit, t2);
		if (tEnter > tExit) {
			return result;
		}
	}

	result.hit = true;
	result.t = tEnter;
	result.point = segment.start + direction * tEnter;
	if (enterAxis == 0) {
		result.normal = Vector2{ enterSign, 0.0f };
	} else if (enterAxis == 1) {
		result.normal = Vector2{ 0.0f, enterSign };
	}
	return result;
}

} // namespace Engine

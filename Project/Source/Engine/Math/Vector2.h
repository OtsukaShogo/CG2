#pragma once
#include <cmath>

namespace Engine {

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 {
	float x;
	float y;

	Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
	Vector2& operator-=(const Vector2& o) { x -= o.x; y -= o.y; return *this; }
	Vector2& operator*=(float s) { x *= s; y *= s; return *this; }
	Vector2& operator/=(float s) { x /= s; y /= s; return *this; }
};

[[nodiscard]] inline Vector2 operator+(const Vector2& a, const Vector2& b) { return { a.x + b.x, a.y + b.y }; }
[[nodiscard]] inline Vector2 operator-(const Vector2& a, const Vector2& b) { return { a.x - b.x, a.y - b.y }; }
[[nodiscard]] inline Vector2 operator-(const Vector2& v) { return { -v.x, -v.y }; }
[[nodiscard]] inline Vector2 operator*(const Vector2& v, float s) { return { v.x * s, v.y * s }; }
[[nodiscard]] inline Vector2 operator*(float s, const Vector2& v) { return { v.x * s, v.y * s }; }
[[nodiscard]] inline Vector2 operator/(const Vector2& v, float s) { return { v.x / s, v.y / s }; }

/// <summary>
/// 完全一致で比較する（誤差を許容したい場合は NearlyEqual を使う）
/// </summary>
[[nodiscard]] inline bool operator==(const Vector2& a, const Vector2& b) { return a.x == b.x && a.y == b.y; }
[[nodiscard]] inline bool operator!=(const Vector2& a, const Vector2& b) { return !(a == b); }

/// <summary>
/// 内積を求める
/// </summary>
[[nodiscard]] inline float Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }

/// <summary>
/// 2次元の外積（z成分のみ）を求める。bがaの左（反時計回り）にあれば正、右なら負
/// </summary>
[[nodiscard]] inline float Cross(const Vector2& a, const Vector2& b) { return a.x * b.y - a.y * b.x; }

/// <summary>
/// 長さの2乗を求める（比較だけならsqrtが要らないのでこちらが速い）
/// </summary>
[[nodiscard]] inline float LengthSquared(const Vector2& v) { return Dot(v, v); }

/// <summary>
/// 長さを求める
/// </summary>
[[nodiscard]] inline float Length(const Vector2& v) { return std::sqrt(Dot(v, v)); }

/// <summary>
/// 2点間の距離を求める
/// </summary>
[[nodiscard]] inline float Distance(const Vector2& a, const Vector2& b) { return Length(b - a); }

/// <summary>
/// 長さ1に正規化する。ゼロベクトル（に近いもの）は、ゼロベクトルのまま返す
/// </summary>
[[nodiscard]] inline Vector2 Normalize(const Vector2& v) {
	float lengthSquared = Dot(v, v);
	if (lengthSquared <= 1e-12f) {
		return { 0.0f, 0.0f };
	}
	return v / std::sqrt(lengthSquared);
}

/// <summary>
/// 線形補間する（t=0でa、t=1でb）
/// </summary>
[[nodiscard]] inline Vector2 Lerp(const Vector2& a, const Vector2& b, float t) {
	return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

/// <summary>
/// 90度反時計回りに回した、垂直なベクトルを返す
/// </summary>
[[nodiscard]] inline Vector2 Perpendicular(const Vector2& v) { return { -v.y, v.x }; }

/// <summary>
/// ベクトルの長さを maxLength 以下に抑える（向きは変えない）
/// </summary>
[[nodiscard]] inline Vector2 ClampLength(const Vector2& v, float maxLength) {
	float lengthSquared = Dot(v, v);
	if (lengthSquared <= maxLength * maxLength || lengthSquared == 0.0f) {
		return v;
	}
	return v * (maxLength / std::sqrt(lengthSquared));
}

/// <summary>
/// ラジアンの角度から、長さ1の方向ベクトルを作る（0で+X、π/2で+Y）
/// </summary>
[[nodiscard]] inline Vector2 FromAngle(float radians) { return { std::cos(radians), std::sin(radians) }; }

/// <summary>
/// ベクトルの向きを角度（ラジアン）で返す。+Xが0、+Yがπ/2
/// </summary>
[[nodiscard]] inline float ToAngle(const Vector2& v) { return std::atan2(v.y, v.x); }

/// <summary>
/// ベクトルを反時計回りに回転させる
/// </summary>
[[nodiscard]] inline Vector2 Rotate(const Vector2& v, float radians) {
	float c = std::cos(radians);
	float s = std::sin(radians);
	return { v.x * c - v.y * s, v.x * s + v.y * c };
}

/// <summary>
/// 許容誤差の範囲内で等しいかを判定する
/// </summary>
[[nodiscard]] inline bool NearlyEqual(const Vector2& a, const Vector2& b, float epsilon = 1e-5f) {
	return std::abs(a.x - b.x) <= epsilon && std::abs(a.y - b.y) <= epsilon;
}

} // namespace Engine

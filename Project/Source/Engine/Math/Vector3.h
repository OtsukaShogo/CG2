#pragma once
#include <algorithm>
#include <cmath>

namespace Engine {

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 {
	float x;
	float y;
	float z;

	Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
};

[[nodiscard]] inline Vector3 operator+(const Vector3& a, const Vector3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
[[nodiscard]] inline Vector3 operator-(const Vector3& a, const Vector3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
[[nodiscard]] inline Vector3 operator-(const Vector3& v) { return { -v.x, -v.y, -v.z }; }
[[nodiscard]] inline Vector3 operator*(const Vector3& v, float s) { return { v.x * s, v.y * s, v.z * s }; }
[[nodiscard]] inline Vector3 operator*(float s, const Vector3& v) { return { v.x * s, v.y * s, v.z * s }; }
[[nodiscard]] inline Vector3 operator/(const Vector3& v, float s) { return { v.x / s, v.y / s, v.z / s }; }

/// <summary>
/// 完全一致で比較する（誤差を許容したい場合は NearlyEqual を使う）
/// </summary>
[[nodiscard]] inline bool operator==(const Vector3& a, const Vector3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
[[nodiscard]] inline bool operator!=(const Vector3& a, const Vector3& b) { return !(a == b); }

/// <summary>
/// 内積を求める
/// </summary>
[[nodiscard]] inline float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

/// <summary>
/// 外積を求める（成分の式は左手系・右手系で共通。左手系では、aからbへ向かって指を曲げたとき、左手の親指の向きが結果になる）
/// </summary>
[[nodiscard]] inline Vector3 Cross(const Vector3& a, const Vector3& b) {
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

/// <summary>
/// 長さの2乗を求める（比較だけならsqrtが要らないのでこちらが速い）
/// </summary>
[[nodiscard]] inline float LengthSquared(const Vector3& v) { return Dot(v, v); }

/// <summary>
/// 長さ（ノルム）を求める
/// </summary>
[[nodiscard]] inline float Length(const Vector3& v) { return std::sqrt(Dot(v, v)); }

/// <summary>
/// 2点間の距離を求める
/// </summary>
[[nodiscard]] inline float Distance(const Vector3& a, const Vector3& b) { return Length(b - a); }

/// <summary>
/// 長さ1に正規化する。ゼロベクトル（に近いもの）は、ゼロベクトルのまま返す
/// </summary>
[[nodiscard]] inline Vector3 Normalize(const Vector3& v) {
	float lengthSquared = Dot(v, v);
	if (lengthSquared <= 1e-12f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return v / std::sqrt(lengthSquared);
}

/// <summary>
/// 線形補間する（t=0でa、t=1でb。tは範囲外でも外挿される）
/// </summary>
[[nodiscard]] inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
	return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

/// <summary>
/// vをontoへ射影したベクトルを求める。ontoがゼロベクトルの場合はゼロベクトルを返す
/// </summary>
[[nodiscard]] inline Vector3 Project(const Vector3& v, const Vector3& onto) {
	float lengthSquared = Dot(onto, onto);
	if (lengthSquared == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return onto * (Dot(v, onto) / lengthSquared);
}

/// <summary>
/// 入射ベクトルを、法線（正規化済み）の面で反射させたベクトルを求める
/// </summary>
[[nodiscard]] inline Vector3 Reflect(const Vector3& input, const Vector3& normal) {
	return input - normal * (2.0f * Dot(input, normal));
}

/// <summary>
/// 与えられたベクトルに垂直なベクトルを1つ返す（長さは保たない。ゼロベクトルならゼロベクトル）
/// </summary>
[[nodiscard]] inline Vector3 Perpendicular(const Vector3& v) {
	if (v.x != 0.0f || v.y != 0.0f) {
		return { -v.y, v.x, 0.0f };
	}
	return { 0.0f, -v.z, v.y };
}

/// <summary>
/// ベクトルの長さを maxLength 以下に抑える（向きは変えない）
/// </summary>
[[nodiscard]] inline Vector3 ClampLength(const Vector3& v, float maxLength) {
	float lengthSquared = Dot(v, v);
	if (lengthSquared <= maxLength * maxLength || lengthSquared == 0.0f) {
		return v;
	}
	return v * (maxLength / std::sqrt(lengthSquared));
}

/// <summary>
/// 成分ごとの最小値を求める
/// </summary>
[[nodiscard]] inline Vector3 Min(const Vector3& a, const Vector3& b) {
	return { (std::min)(a.x, b.x), (std::min)(a.y, b.y), (std::min)(a.z, b.z) };
}

/// <summary>
/// 成分ごとの最大値を求める
/// </summary>
[[nodiscard]] inline Vector3 Max(const Vector3& a, const Vector3& b) {
	return { (std::max)(a.x, b.x), (std::max)(a.y, b.y), (std::max)(a.z, b.z) };
}

/// <summary>
/// 許容誤差の範囲内で等しいかを判定する
/// </summary>
[[nodiscard]] inline bool NearlyEqual(const Vector3& a, const Vector3& b, float epsilon = 1e-5f) {
	return std::abs(a.x - b.x) <= epsilon && std::abs(a.y - b.y) <= epsilon && std::abs(a.z - b.z) <= epsilon;
}

} // namespace Engine

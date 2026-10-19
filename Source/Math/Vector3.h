#pragma once

namespace Engine {

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 {
	float x;
	float y;
	float z;

	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
};

/// <summary>
/// 2つのベクトルの和を返す
/// </summary>
/// <param name="a">左辺のベクトル</param>
/// <param name="b">右辺のベクトル</param>
/// <returns>各成分を足したベクトル</returns>
[[nodiscard]] inline Vector3 operator+(const Vector3& a, const Vector3& b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

/// <summary>
/// 各成分をスカラー倍したベクトルを返す
/// </summary>
/// <param name="v">対象のベクトル</param>
/// <param name="scalar">乗算するスカラー値</param>
/// <returns>スカラー倍したベクトル</returns>
[[nodiscard]] inline Vector3 operator*(const Vector3& v, float scalar) {
	return { v.x * scalar, v.y * scalar, v.z * scalar };
}

} // namespace Engine

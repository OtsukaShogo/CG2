#include "Quaternion.h"
#include "MathUtil.h"
#include "TransformMatrix.h"
#include <cmath>

namespace Engine {

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	Quaternion result;
	result.x = lhs.y * rhs.z - lhs.z * rhs.y + rhs.w * lhs.x + lhs.w * rhs.x;
	result.y = lhs.z * rhs.x - lhs.x * rhs.z + rhs.w * lhs.y + lhs.w * rhs.y;
	result.z = lhs.x * rhs.y - lhs.y * rhs.x + rhs.w * lhs.z + lhs.w * rhs.z;
	result.w = lhs.w * rhs.w - (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
	return result;
}

Quaternion Conjugate(const Quaternion& q) {
	return { -q.x, -q.y, -q.z, q.w };
}

float Dot(const Quaternion& a, const Quaternion& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float Norm(const Quaternion& q) {
	return std::sqrt(Dot(q, q));
}

Quaternion Normalize(const Quaternion& q) {
	float norm = Norm(q);
	if (norm == 0.0f) {
		return IdentityQuaternion(); // 向きを持たないので、回転なしとして扱う
	}
	return { q.x / norm, q.y / norm, q.z / norm, q.w / norm };
}

Quaternion Inverse(const Quaternion& q) {
	float normSquared = Dot(q, q);
	if (normSquared == 0.0f) {
		return IdentityQuaternion();
	}
	Quaternion conjugate = Conjugate(q);
	return { conjugate.x / normSquared, conjugate.y / normSquared, conjugate.z / normSquared, conjugate.w / normSquared };
}

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	Vector3 n = Normalize(axis);
	if (n == Vector3{ 0.0f, 0.0f, 0.0f }) {
		return IdentityQuaternion(); // 軸が定まらない
	}
	float half = angle * 0.5f;
	float s = std::sin(half);
	return { n.x * s, n.y * s, n.z * s, std::cos(half) };
}

Quaternion MakeQuaternionFromEuler(const Vector3& euler) {
	// 行列は Rx * Ry * Rz（行ベクトル規約でX→Y→Zの順に適用）。クォータニオンでは先に適用するものを右に置く
	Quaternion qx = MakeRotateAxisAngleQuaternion({ 1.0f, 0.0f, 0.0f }, euler.x);
	Quaternion qy = MakeRotateAxisAngleQuaternion({ 0.0f, 1.0f, 0.0f }, euler.y);
	Quaternion qz = MakeRotateAxisAngleQuaternion({ 0.0f, 0.0f, 1.0f }, euler.z);
	return Multiply(qz, Multiply(qy, qx));
}

Quaternion DirectionToDirection(const Vector3& from, const Vector3& to) {
	Vector3 f = Normalize(from);
	Vector3 t = Normalize(to);

	float dot = Dot(f, t);
	Vector3 cross = Cross(f, t);
	float sinLength = Length(cross);

	// どちらかがゼロベクトル、または同じ向き: 回転なし
	if (sinLength < 1e-6f && dot >= 0.0f) {
		return IdentityQuaternion();
	}

	// 逆向き(180度回転): fromに垂直な任意の軸で回す
	if (sinLength < 1e-6f) {
		return MakeRotateAxisAngleQuaternion(Perpendicular(f), kPi);
	}

	// 軸 = cross、角度 = atan2(sin, cos)
	return MakeRotateAxisAngleQuaternion(cross, std::atan2(sinLength, dot));
}

Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
	Quaternion a = q0;
	float dot = Dot(a, q1);
	if (dot < 0.0f) {
		// 反対側の回転を使うと、短いほうの経路を通る
		a = { -a.x, -a.y, -a.z, -a.w };
		dot = -dot;
	}

	float scale0, scale1;
	if (dot >= 1.0f - 1e-6f) {
		// ほぼ同じ回転: sin(theta)が0に近いので、線形補間で代用する（0除算を避ける）
		scale0 = 1.0f - t;
		scale1 = t;
	} else {
		float theta = std::acos(dot);
		float sinTheta = std::sin(theta);
		scale0 = std::sin((1.0f - t) * theta) / sinTheta;
		scale1 = std::sin(t * theta) / sinTheta;
	}

	// 線形補間の場合は長さが1からずれるので、正規化して返す
	return Normalize({
		scale0 * a.x + scale1 * q1.x,
		scale0 * a.y + scale1 * q1.y,
		scale0 * a.z + scale1 * q1.z,
		scale0 * a.w + scale1 * q1.w,
	});
}

Vector3 RotateVector(const Vector3& vector, const Quaternion& q) {
	Quaternion r = { vector.x, vector.y, vector.z, 0.0f };
	Quaternion result = Multiply(Multiply(q, r), Conjugate(q));
	return { result.x, result.y, result.z };
}

Matrix4x4 MakeRotateMatrix(const Quaternion& q) {
	float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z, ww = q.w * q.w;
	float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
	float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;

	Matrix4x4 m = {};
	m.m[0][0] = ww + xx - yy - zz;
	m.m[0][1] = 2.0f * (xy + wz);
	m.m[0][2] = 2.0f * (xz - wy);

	m.m[1][0] = 2.0f * (xy - wz);
	m.m[1][1] = ww - xx + yy - zz;
	m.m[1][2] = 2.0f * (yz + wx);

	m.m[2][0] = 2.0f * (xz + wy);
	m.m[2][1] = 2.0f * (yz - wx);
	m.m[2][2] = ww - xx - yy + zz;

	m.m[3][3] = 1.0f;

	return m;
}

Matrix4x4 MakeAffineMatrixFromQuaternion(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
	return Multiply(Multiply(MakeScaleMatrix(scale), MakeRotateMatrix(rotate)), MakeTranslateMatrix(translate));
}

} // namespace Engine

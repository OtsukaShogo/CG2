#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"

namespace Engine {

/// <summary>
/// クォータニオン（回転を表す）。何も指定しない場合は「回転なし」になる。
/// 回転の向きは、このエンジンの回転行列（MakeRotateX/Y/ZMatrix）と同じ
/// </summary>
struct Quaternion {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;
};

/// <summary>
/// 回転なしを表すクォータニオン（乗法単位元）を返す
/// </summary>
[[nodiscard]] inline Quaternion IdentityQuaternion() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }

/// <summary>
/// 2つのクォータニオンの積を求める。
/// 回転を合成するときは、先に適用する回転を右側に置く（Multiply(後, 先)）
/// </summary>
[[nodiscard]] Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);

/// <summary>
/// 共役クォータニオンを求める（単位クォータニオンなら逆回転になる）
/// </summary>
[[nodiscard]] Quaternion Conjugate(const Quaternion& q);

/// <summary>
/// ノルム（大きさ）を求める
/// </summary>
[[nodiscard]] float Norm(const Quaternion& q);

/// <summary>
/// 内積を求める
/// </summary>
[[nodiscard]] float Dot(const Quaternion& a, const Quaternion& b);

/// <summary>
/// 正規化する（長さ1にする）。ノルムが0の場合は回転なし（単位クォータニオン）を返す
/// </summary>
[[nodiscard]] Quaternion Normalize(const Quaternion& q);

/// <summary>
/// 逆クォータニオンを求める。ノルムが0の場合は回転なしを返す
/// </summary>
[[nodiscard]] Quaternion Inverse(const Quaternion& q);

/// <summary>
/// 任意軸まわりの回転を表すクォータニオンを作成する
/// </summary>
/// <param name="axis">回転軸（正規化されていなくてよい。ゼロベクトルなら回転なし）</param>
/// <param name="angle">回転角（ラジアン）</param>
[[nodiscard]] Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);

/// <summary>
/// オイラー角（X→Y→Zの順に適用。MakeAffineMatrixと同じ）から、クォータニオンを作成する
/// </summary>
/// <param name="euler">各軸の回転角（ラジアン）</param>
[[nodiscard]] Quaternion MakeQuaternionFromEuler(const Vector3& euler);

/// <summary>
/// fromの向きをtoの向きへ回す、最短の回転を表すクォータニオンを作成する
/// </summary>
/// <param name="from">元の向き（正規化されていなくてよい）</param>
/// <param name="to">向けたい向き（正規化されていなくてよい）</param>
[[nodiscard]] Quaternion DirectionToDirection(const Vector3& from, const Vector3& to);

/// <summary>
/// 球面線形補間する（常に短いほうの経路を通り、結果は正規化される）
/// </summary>
/// <param name="q0">t=0 のときの回転</param>
/// <param name="q1">t=1 のときの回転</param>
/// <param name="t">補間係数（0〜1）</param>
[[nodiscard]] Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);

/// <summary>
/// ベクトルをクォータニオンで回転させる
/// </summary>
[[nodiscard]] Vector3 RotateVector(const Vector3& vector, const Quaternion& q);

/// <summary>
/// クォータニオンから回転行列を作成する
/// </summary>
[[nodiscard]] Matrix4x4 MakeRotateMatrix(const Quaternion& q);

/// <summary>
/// 拡大縮小・回転（クォータニオン）・平行移動から、アフィン変換行列を作成する。
/// （MakeAffineMatrix のオーバーロードにしないのは、{x,y,z} の初期化子リストで呼んだときに曖昧にならないようにするため）
/// </summary>
[[nodiscard]] Matrix4x4 MakeAffineMatrixFromQuaternion(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);

[[nodiscard]] inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) { return Multiply(lhs, rhs); }

} // namespace Engine

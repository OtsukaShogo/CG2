#pragma once

namespace Engine {

/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4 {
	float m[4][4];
};

//=== 行列の演算関数の宣言 ===================================

/// <summary>
/// 2つの行列の和を求める
/// </summary>
/// <param name="m1">行列1</param>
/// <param name="m2">行列2</param>
/// <returns>2つの行列の和</returns>
[[nodiscard]] Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

/// <summary>
/// 2つの行列の差を求める
/// </summary>
/// <param name="m1">行列1</param>
/// <param name="m2">行列2</param>
/// <returns>2つの行列の差</returns>
[[nodiscard]] Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

/// <summary>
/// 2つの行列の積を求める
/// </summary>
/// <param name="m1">行列1</param>
/// <param name="m2">行列2</param>
/// <returns>2つの行列の積</returns>
[[nodiscard]] Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

/// <summary>
/// 行列の逆行列を求める（存在しない場合は単位行列を返す）
/// </summary>
/// <param name="m">逆行列を求める行列</param>
/// <returns>逆行列（存在しない場合は単位行列）</returns>
[[nodiscard]] Matrix4x4 Inverse(const Matrix4x4& m);

/// <summary>
/// 行列の転置行列を求める
/// </summary>
/// <param name="m">転置する行列</param>
/// <returns>転置行列</returns>
[[nodiscard]] Matrix4x4 Transpose(const Matrix4x4& m);

/// <summary>
/// 単位行列を作成する
/// </summary>
/// <returns>単位行列</returns>
[[nodiscard]] Matrix4x4 MakeIdentity4x4();

} // namespace Engine

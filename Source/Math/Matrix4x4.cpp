#include"Matrix4x4.h"

namespace Engine {

//=== 行列の演算関数の定義 ===================================

//行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

//行列の減法
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

//行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

namespace {

/// <summary>
/// 3x3行列式を求める（余因子計算に使用する内部ヘルパー）
/// </summary>
float Determinant3x3(
	float a, float b, float c,
	float d, float e, float f,
	float g, float h, float i) {
	return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

/// <summary>
/// 4x4行列mから指定した行・列を除いた3x3小行列式（余因子の絶対値部分）を求める
/// </summary>
float Minor(const Matrix4x4& m, int row, int col) {
	float v[9];
	int index = 0;
	for (int r = 0; r < 4; ++r) {
		if (r == row) continue;
		for (int c = 0; c < 4; ++c) {
			if (c == col) continue;
			v[index++] = m.m[r][c];
		}
	}
	return Determinant3x3(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
}

/// <summary>
/// 4x4行列mの符号付き余因子 C(row,col) = (-1)^(row+col) * Minor(row,col) を求める
/// </summary>
float Cofactor(const Matrix4x4& m, int row, int col) {
	float sign = ((row + col) % 2 == 0) ? 1.0f : -1.0f;
	return sign * Minor(m, row, col);
}

} // namespace

//逆行列
// 逆行列 = 随伴行列（余因子行列の転置） / 行列式 という定義をそのまま実装している。
// 行列式は1行目に沿った余因子展開で求める。
Matrix4x4 Inverse(const Matrix4x4& m) {
	// 行列式（1行目に沿った余因子展開）
	float det = 0.0f;
	for (int col = 0; col < 4; ++col) {
		det += m.m[0][col] * Cofactor(m, 0, col);
	}

	if (det == 0.0f) {
		return MakeIdentity4x4();//逆行列が存在しない場合は単位行列を返す
	}

	float invDet = 1.0f / det;

	// 各成分 = 随伴行列（余因子行列の転置） × 1/det
	// result.m[row][col] = Cofactor(m, col, row) となる点に注意（転置されるため行と列が入れ替わる）
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = Cofactor(m, col, row) * invDet;
		}
	}

	return result;
}

//転置行列
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

//単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1.0f;
	}
	return result;
}

} // namespace Engine
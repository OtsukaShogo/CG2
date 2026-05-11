#include "TransformMatrix.h"
#include"Matrix4x4.h"
#include"Vector3.h"
#include<cassert>
#include<cmath>

//平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

//拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;

	return result;
}

//X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	return result;
}

//Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = std::cos(radian);
	result.m[0][2] = -std::sin(radian);
	result.m[2][0] = std::sin(radian);
	result.m[2][2] = std::cos(radian);
	return result;
}

//Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result = MakeIdentity4x4();
	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	return result;
}

//座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	assert(w != 0.0f);

	result.x /= w;
	result.y /= w;
	result.z /= w;

	return result;
}

//3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	//スケール行列
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);

	//回転行列
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	Matrix4x4 rotateMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

	//平行移動行列
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	//合成(S→R→Tの順)
	Matrix4x4 result = Multiply(scaleMatrix, rotateMatrix);
	result = Multiply(result, translateMatrix);

	return result;
}

//射影投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {};

	float f = 1.0f / (tanf(fovY * 0.5f));
	float rangeInverse = 1.0f / (farClip - nearClip);

	//射影行列
	result.m[0][0] = (1.0f / aspectRatio) * f;
	result.m[1][1] = f;
	result.m[2][2] = farClip * rangeInverse;
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / rangeInverse;

	return result;
}

//正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};

	//幅・高さ・奥行きの逆数
	float widthInverse = 1.0f / (right - left);
	float heightInverse = 1.0f / (top - bottom);
	float depthInverse = 1.0f / (farClip - nearClip);

	//スケール
	result.m[0][0] = 2.0f * widthInverse;
	result.m[1][1] = 2.0f * heightInverse;
	result.m[2][2] = depthInverse;

	//平行移動
	result.m[3][0] = -(right + left) * widthInverse;
	result.m[3][1] = -(top + bottom) * heightInverse;
	result.m[3][2] = -nearClip * depthInverse;
	result.m[3][3] = 1.0f;

	return result;
}

//ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};

	// スケール
	result.m[0][0] = width * 0.5f;
	result.m[1][1] = -(height * 0.5f);
	result.m[2][2] = maxDepth - minDepth;

	// 平行移動
	result.m[3][0] = left + (width * 0.5f);
	result.m[3][1] = top + (height * 0.5f);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;

	return result;
}
#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"

namespace Engine {

/// <summary>
/// ワールド・ビュー・プロジェクション変換行列をまとめた定数バッファ用データ
/// </summary>
struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

/// <summary>
/// 平行移動行列を作成する
/// </summary>
/// <param name="translate">平行移動量</param>
/// <returns>平行移動行列</returns>
[[nodiscard]] Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

/// <summary>
/// 拡大縮小行列を作成する
/// </summary>
/// <param name="scale">拡大縮小率</param>
/// <returns>拡大縮小行列</returns>
[[nodiscard]] Matrix4x4 MakeScaleMatrix(const Vector3& scale);

/// <summary>
/// X軸回転行列を作成する
/// </summary>
/// <param name="radian">回転角（ラジアン）</param>
/// <returns>X軸回転行列</returns>
[[nodiscard]] Matrix4x4 MakeRotateXMatrix(float radian);

/// <summary>
/// Y軸回転行列を作成する
/// </summary>
/// <param name="radian">回転角（ラジアン）</param>
/// <returns>Y軸回転行列</returns>
[[nodiscard]] Matrix4x4 MakeRotateYMatrix(float radian);

/// <summary>
/// Z軸回転行列を作成する
/// </summary>
/// <param name="radian">回転角（ラジアン）</param>
/// <returns>Z軸回転行列</returns>
[[nodiscard]] Matrix4x4 MakeRotateZMatrix(float radian);

/// <summary>
/// ベクトルを行列で座標変換する
/// </summary>
/// <param name="vector">変換するベクトル</param>
/// <param name="matrix">適用する行列</param>
/// <returns>変換後のベクトル</returns>
[[nodiscard]] Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

/// <summary>
/// 拡大縮小・回転・平行移動をまとめた3次元アフィン変換行列を作成する
/// </summary>
/// <param name="scale">拡大縮小率</param>
/// <param name="rotate">回転角（XYZ各軸、ラジアン）</param>
/// <param name="translate">平行移動量</param>
/// <returns>アフィン変換行列</returns>
[[nodiscard]] Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

/// <summary>
/// 透視投影行列を作成する
/// </summary>
/// <param name="fovY">縦方向の視野角（ラジアン）</param>
/// <param name="aspectRatio">アスペクト比（幅/高さ）</param>
/// <param name="nearClip">ニアクリップ距離</param>
/// <param name="farClip">ファークリップ距離</param>
/// <returns>透視投影行列</returns>
[[nodiscard]] Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

/// <summary>
/// 正射影行列を作成する
/// </summary>
/// <param name="left">クリップ空間の左端</param>
/// <param name="top">クリップ空間の上端</param>
/// <param name="right">クリップ空間の右端</param>
/// <param name="bottom">クリップ空間の下端</param>
/// <param name="nearClip">ニアクリップ距離</param>
/// <param name="farClip">ファークリップ距離</param>
/// <returns>正射影行列</returns>
[[nodiscard]] Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

/// <summary>
/// ビューポート変換行列を作成する
/// </summary>
/// <param name="left">ビューポート左端座標</param>
/// <param name="top">ビューポート上端座標</param>
/// <param name="width">ビューポート幅</param>
/// <param name="height">ビューポート高さ</param>
/// <param name="minDepth">最小深度値</param>
/// <param name="maxDepth">最大深度値</param>
/// <returns>ビューポート変換行列</returns>
[[nodiscard]] Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

} // namespace Engine

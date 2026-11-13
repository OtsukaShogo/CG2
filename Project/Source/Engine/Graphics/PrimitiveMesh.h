#pragma once
#include <cstdint>
#include <vector>
#include "VertexData.h"
#include "../Math/Vector3.h"

namespace Engine {

/// <summary>
/// CPU側の頂点・インデックスデータ（GPUへのアップロード前の形）
/// </summary>
struct MeshData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

/// <summary>
/// 基本図形の種類（ModelData::CreatePrimitive で使う）。
/// 大きさは下記の「基準サイズ」で作られるので、Modelのスケールで拡大縮小する
/// </summary>
enum class PrimitiveType {
	kSphere,   // 半径1の球（直径2）
	kBox,      // 一辺1の立方体
	kPlane,    // 1x1の水平な板（XZ平面、上向き）
	kCylinder, // 半径0.5・高さ1の円柱（Y軸方向）
	kCone,     // 底面の半径0.5・高さ1の円錐（頂点が+Y側）
};

// 以下の関数が作る図形は、すべて原点を中心とし、左手系・Y上向き。
// 面の表は、D3D12の既定（時計回りが表）に合わせてあり、外側から見える。
// 法線は外向き、UVは各面で0〜1。

/// <summary>
/// 球を作る（半径1）
/// </summary>
/// <param name="subdivision">緯度・経度の分割数（3未満は3にする）</param>
[[nodiscard]] MeshData MakeSphereMeshData(uint32_t subdivision = 16);

/// <summary>
/// XY平面上の矩形を作る（左上が原点、+Yが下。Sprite用）
/// </summary>
[[nodiscard]] MeshData MakeRectMeshData(float width, float height);

/// <summary>
/// 直方体を作る
/// </summary>
/// <param name="size">X・Y・Zそれぞれの長さ</param>
[[nodiscard]] MeshData MakeBoxMeshData(const Vector3& size = { 1.0f, 1.0f, 1.0f });

/// <summary>
/// 水平な板（XZ平面、法線は+Y）を作る
/// </summary>
/// <param name="width">X方向の長さ</param>
/// <param name="depth">Z方向の長さ</param>
[[nodiscard]] MeshData MakePlaneMeshData(float width = 1.0f, float depth = 1.0f);

/// <summary>
/// 円柱（Y軸方向）を作る
/// </summary>
/// <param name="radius">半径</param>
/// <param name="height">高さ</param>
/// <param name="segments">円周の分割数（3未満は3にする）</param>
[[nodiscard]] MeshData MakeCylinderMeshData(float radius = 0.5f, float height = 1.0f, uint32_t segments = 24);

/// <summary>
/// 円錐（頂点が+Y側）を作る
/// </summary>
/// <param name="radius">底面の半径</param>
/// <param name="height">高さ</param>
/// <param name="segments">円周の分割数（3未満は3にする）</param>
[[nodiscard]] MeshData MakeConeMeshData(float radius = 0.5f, float height = 1.0f, uint32_t segments = 24);

/// <summary>
/// 種類を指定して、基準サイズの図形を作る
/// </summary>
[[nodiscard]] MeshData MakePrimitiveMeshData(PrimitiveType type);

} // namespace Engine

#include "PrimitiveMesh.h"
#include "../Math/MathUtil.h"
#include <algorithm>
#include <cmath>

namespace Engine {

namespace {

VertexData MakeVertex(const Vector3& position, const Vector3& normal, float u, float v) {
	VertexData vertex{};
	vertex.position = { position.x, position.y, position.z, 1.0f };
	vertex.texcoord = { u, v };
	vertex.normal = normal;
	return vertex;
}

// 4頂点を、(0,1,2) と (0,2,3) の2つの三角形として追加する。
// 頂点は「外から見て、左下→左上→右上→右下」の順に渡す（この順だと時計回りで表になる）
void AddQuad(MeshData& mesh, const VertexData& a, const VertexData& b, const VertexData& c, const VertexData& d) {
	uint32_t start = static_cast<uint32_t>(mesh.vertices.size());
	mesh.vertices.push_back(a);
	mesh.vertices.push_back(b);
	mesh.vertices.push_back(c);
	mesh.vertices.push_back(d);
	for (uint32_t i : { 0u, 1u, 2u, 0u, 2u, 3u }) {
		mesh.indices.push_back(start + i);
	}
}

// 法線normal・「外から見て右」の向きu・「外から見て上」の向きv（v = u × normal）の面（半幅halfU・半高さhalfV）を追加する
void AddFace(MeshData& mesh, const Vector3& center, const Vector3& normal, const Vector3& u, float halfU, const Vector3& v, float halfV) {
	auto corner = [&](float su, float sv) {
		return Vector3{ center.x + u.x * halfU * su + v.x * halfV * sv,
		                center.y + u.y * halfU * su + v.y * halfV * sv,
		                center.z + u.z * halfU * su + v.z * halfV * sv };
	};
	AddQuad(mesh,
		MakeVertex(corner(-1, -1), normal, 0.0f, 1.0f),
		MakeVertex(corner(-1, +1), normal, 0.0f, 0.0f),
		MakeVertex(corner(+1, +1), normal, 1.0f, 0.0f),
		MakeVertex(corner(+1, -1), normal, 1.0f, 1.0f));
}

Vector3 Cross3(const Vector3& a, const Vector3& b) {
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

// 円周上の点（Y軸まわり。角度0が+X、角度が増えると+Zへ回る）
Vector3 RingPoint(float radius, float angle, float y) {
	return { radius * std::cos(angle), y, radius * std::sin(angle) };
}

// 円盤（円柱の上下・円錐の底）を追加する。upward=trueなら法線は+Y
void AddDisk(MeshData& mesh, float radius, float y, uint32_t segments, bool upward) {
	const float step = kTwoPi / static_cast<float>(segments);
	const Vector3 normal = { 0.0f, upward ? 1.0f : -1.0f, 0.0f };
	// UVは、円を0〜1の正方形に収めて割り当てる
	auto makeRingVertex = [&](float angle) {
		Vector3 p = RingPoint(radius, angle, y);
		return MakeVertex(p, normal, 0.5f + p.x / (radius * 2.0f), 0.5f - p.z / (radius * 2.0f));
	};
	for (uint32_t i = 0; i < segments; ++i) {
		float a0 = step * static_cast<float>(i);
		float a1 = step * static_cast<float>(i + 1);
		uint32_t start = static_cast<uint32_t>(mesh.vertices.size());
		mesh.vertices.push_back(MakeVertex({ 0.0f, y, 0.0f }, normal, 0.5f, 0.5f));
		// 外から見て時計回りが表（上向きは θ が減る向き、下向きは増える向き）
		if (upward) {
			mesh.vertices.push_back(makeRingVertex(a1));
			mesh.vertices.push_back(makeRingVertex(a0));
		} else {
			mesh.vertices.push_back(makeRingVertex(a0));
			mesh.vertices.push_back(makeRingVertex(a1));
		}
		mesh.indices.push_back(start);
		mesh.indices.push_back(start + 1);
		mesh.indices.push_back(start + 2);
	}
}

Vector3 Normalized(const Vector3& v) {
	float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return length > 0.0f ? Vector3{ v.x / length, v.y / length, v.z / length } : Vector3{ 0.0f, 1.0f, 0.0f };
}

} // namespace

MeshData MakeSphereMeshData(uint32_t subdivision) {
	subdivision = (std::max)(subdivision, 3u);

	// 緯度・経度1区画（クアッド）あたりの頂点数・インデックス数（三角形2枚分）
	constexpr uint32_t kVerticesPerQuad = 4;
	constexpr uint32_t kIndicesPerQuad = 6;

	const float kLonEvery = kTwoPi / float(subdivision);
	const float kLatEvery = kPi / float(subdivision);

	MeshData mesh;
	mesh.vertices.reserve(subdivision * subdivision * kVerticesPerQuad);
	mesh.indices.reserve(subdivision * subdivision * kIndicesPerQuad);

	auto point = [](float lat, float lon) {
		return Vector3{ std::cos(lat) * std::cos(lon), std::sin(lat), std::cos(lat) * std::sin(lon) };
	};

	// 緯度・経度をsubdivision分割し、各区画（クアッド）ごとに4頂点・2三角形を生成してUV球を作る
	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = -kPi / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			float startU = float(lonIndex) / float(subdivision);
			float startV = 1.0f - float(latIndex) / float(subdivision);
			float nextU = float(lonIndex + 1) / float(subdivision);
			float nextV = 1.0f - float(latIndex + 1) / float(subdivision);

			Vector3 pa = point(lat, lon);
			Vector3 pb = point(lat + kLatEvery, lon);
			Vector3 pc = point(lat, lon + kLonEvery);
			Vector3 pd = point(lat + kLatEvery, lon + kLonEvery);

			uint32_t vStart = static_cast<uint32_t>(mesh.vertices.size());
			// 半径1の球なので、法線は座標と同じ
			mesh.vertices.push_back(MakeVertex(pa, pa, startU, startV)); // a
			mesh.vertices.push_back(MakeVertex(pb, pb, startU, nextV));  // b
			mesh.vertices.push_back(MakeVertex(pc, pc, nextU, startV));  // c
			mesh.vertices.push_back(MakeVertex(pd, pd, nextU, nextV));   // d

			// T1: a, b, c
			mesh.indices.push_back(vStart + 0);
			mesh.indices.push_back(vStart + 1);
			mesh.indices.push_back(vStart + 2);

			// T2: d, c, b
			mesh.indices.push_back(vStart + 3);
			mesh.indices.push_back(vStart + 2);
			mesh.indices.push_back(vStart + 1);
		}
	}
	return mesh;
}

MeshData MakeRectMeshData(float width, float height) {
	MeshData mesh;
	mesh.vertices.push_back(MakeVertex({ 0.0f, height, 0.0f }, { 0.0f, 0.0f, -1.0f }, 0.0f, 1.0f)); // 左下
	mesh.vertices.push_back(MakeVertex({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, 0.0f, 0.0f));   // 左上
	mesh.vertices.push_back(MakeVertex({ width, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, 1.0f, 0.0f));  // 右上
	mesh.vertices.push_back(MakeVertex({ width, height, 0.0f }, { 0.0f, 0.0f, -1.0f }, 1.0f, 1.0f)); // 右下
	mesh.indices = { 0, 1, 3, 1, 2, 3 };
	return mesh;
}

MeshData MakeBoxMeshData(const Vector3& size) {
	const float hx = size.x * 0.5f, hy = size.y * 0.5f, hz = size.z * 0.5f;
	MeshData mesh;
	mesh.vertices.reserve(24);
	mesh.indices.reserve(36);

	// 各面: 法線n、外から見て右u、半幅（uの向き）・半高さ（vの向き。v = u × n）
	struct Face { Vector3 n, u; float halfU, halfV; };
	const Face faces[] = {
		{ { +1, 0, 0 }, { 0, 0, +1 }, hz, hy }, // +X
		{ { -1, 0, 0 }, { 0, 0, -1 }, hz, hy }, // -X
		{ { 0, +1, 0 }, { +1, 0, 0 }, hx, hz }, // +Y
		{ { 0, -1, 0 }, { +1, 0, 0 }, hx, hz }, // -Y
		{ { 0, 0, +1 }, { -1, 0, 0 }, hx, hy }, // +Z
		{ { 0, 0, -1 }, { +1, 0, 0 }, hx, hy }, // -Z
	};
	for (const Face& f : faces) {
		Vector3 v = Cross3(f.u, f.n);
		Vector3 center = { f.n.x * hx, f.n.y * hy, f.n.z * hz };
		AddFace(mesh, center, f.n, f.u, f.halfU, v, f.halfV);
	}
	return mesh;
}

MeshData MakePlaneMeshData(float width, float depth) {
	MeshData mesh;
	// 上から見て、右が+X・上が+Z
	AddFace(mesh, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, width * 0.5f, { 0.0f, 0.0f, 1.0f }, depth * 0.5f);
	return mesh;
}

MeshData MakeCylinderMeshData(float radius, float height, uint32_t segments) {
	segments = (std::max)(segments, 3u);
	const float half = height * 0.5f;
	const float step = kTwoPi / static_cast<float>(segments);
	MeshData mesh;

	// 側面（外から見て、右が θ の増える向き、上が +Y）
	for (uint32_t i = 0; i < segments; ++i) {
		float a0 = step * static_cast<float>(i);
		float a1 = step * static_cast<float>(i + 1);
		Vector3 n0 = { std::cos(a0), 0.0f, std::sin(a0) };
		Vector3 n1 = { std::cos(a1), 0.0f, std::sin(a1) };
		float u0 = static_cast<float>(i) / static_cast<float>(segments);
		float u1 = static_cast<float>(i + 1) / static_cast<float>(segments);
		AddQuad(mesh,
			MakeVertex(RingPoint(radius, a0, -half), n0, u0, 1.0f),
			MakeVertex(RingPoint(radius, a0, +half), n0, u0, 0.0f),
			MakeVertex(RingPoint(radius, a1, +half), n1, u1, 0.0f),
			MakeVertex(RingPoint(radius, a1, -half), n1, u1, 1.0f));
	}
	AddDisk(mesh, radius, +half, segments, true);
	AddDisk(mesh, radius, -half, segments, false);
	return mesh;
}

MeshData MakeConeMeshData(float radius, float height, uint32_t segments) {
	segments = (std::max)(segments, 3u);
	const float half = height * 0.5f;
	const float step = kTwoPi / static_cast<float>(segments);
	MeshData mesh;

	// 側面の法線は、斜面に垂直な向き（水平成分 = height、垂直成分 = radius の比）
	auto sideNormal = [&](float angle) {
		return Normalized({ height * std::cos(angle), radius, height * std::sin(angle) });
	};

	for (uint32_t i = 0; i < segments; ++i) {
		float a0 = step * static_cast<float>(i);
		float a1 = step * static_cast<float>(i + 1);
		float aMid = (a0 + a1) * 0.5f;
		float u0 = static_cast<float>(i) / static_cast<float>(segments);
		float u1 = static_cast<float>(i + 1) / static_cast<float>(segments);
		uint32_t start = static_cast<uint32_t>(mesh.vertices.size());
		// 先端→右下→左下の順が、外から見て時計回り
		mesh.vertices.push_back(MakeVertex({ 0.0f, +half, 0.0f }, sideNormal(aMid), (u0 + u1) * 0.5f, 0.0f));
		mesh.vertices.push_back(MakeVertex(RingPoint(radius, a1, -half), sideNormal(a1), u1, 1.0f));
		mesh.vertices.push_back(MakeVertex(RingPoint(radius, a0, -half), sideNormal(a0), u0, 1.0f));
		mesh.indices.push_back(start);
		mesh.indices.push_back(start + 1);
		mesh.indices.push_back(start + 2);
	}
	AddDisk(mesh, radius, -half, segments, false);
	return mesh;
}

MeshData MakePrimitiveMeshData(PrimitiveType type) {
	switch (type) {
	case PrimitiveType::kBox:      return MakeBoxMeshData();
	case PrimitiveType::kPlane:    return MakePlaneMeshData();
	case PrimitiveType::kCylinder: return MakeCylinderMeshData();
	case PrimitiveType::kCone:     return MakeConeMeshData();
	case PrimitiveType::kSphere:
	default:                       return MakeSphereMeshData();
	}
}

} // namespace Engine

#include "TestFramework.h"
#include "Source/Engine/Graphics/PrimitiveMesh.h"
#include "Source/Engine/Math/MathUtil.h"
#include "Source/Engine/Math/Vector3.h"

#include <algorithm>
#include <cmath>

using namespace Engine;

namespace {

Vector3 Pos(const VertexData& v) { return Vector3{ v.position.x, v.position.y, v.position.z }; }

// 三角形の外向き判定用ベクトル（左手系・時計回りが表のとき、法線の向きを向く）
Vector3 TriangleCross(const MeshData& mesh, size_t triangle) {
	Vector3 p0 = Pos(mesh.vertices[mesh.indices[triangle * 3 + 0]]);
	Vector3 p1 = Pos(mesh.vertices[mesh.indices[triangle * 3 + 1]]);
	Vector3 p2 = Pos(mesh.vertices[mesh.indices[triangle * 3 + 2]]);
	return Cross(Vector3{ p1 - p0 }, Vector3{ p2 - p0 });
}

// 形が壊れていないことの共通チェック（インデックスの範囲・法線の長さ・UVの範囲・表向きの向き）
void CheckWellFormed(const MeshData& mesh) {
	CHECK(!mesh.vertices.empty());
	CHECK(mesh.indices.size() % 3 == 0);

	bool indicesInRange = true;
	for (uint32_t index : mesh.indices) {
		indicesInRange = indicesInRange && index < mesh.vertices.size();
	}
	CHECK(indicesInRange);
	if (!indicesInRange) {
		return;
	}

	bool normalsOk = true, uvOk = true, finiteOk = true;
	for (const VertexData& v : mesh.vertices) {
		normalsOk = normalsOk && std::abs(Length(v.normal) - 1.0f) < 1e-4f;
		uvOk = uvOk && v.texcoord.x >= -1e-5f && v.texcoord.x <= 1.0f + 1e-5f && v.texcoord.y >= -1e-5f && v.texcoord.y <= 1.0f + 1e-5f;
		finiteOk = finiteOk && std::isfinite(v.position.x) && std::isfinite(v.position.y) && std::isfinite(v.position.z);
	}
	CHECK(normalsOk);
	CHECK(uvOk);
	CHECK(finiteOk);

	// 面積のある三角形は、時計回り（外から見て）で、頂点の法線と同じ向きを向く
	bool facingOk = true;
	for (size_t t = 0; t < mesh.indices.size() / 3; ++t) {
		Vector3 c = TriangleCross(mesh, t);
		if (Length(c) < 1e-6f) {
			continue; // 球の極にできる、面積のない三角形
		}
		Vector3 n = mesh.vertices[mesh.indices[t * 3]].normal + mesh.vertices[mesh.indices[t * 3 + 1]].normal + mesh.vertices[mesh.indices[t * 3 + 2]].normal;
		facingOk = facingOk && Dot(c, n) > 0.0f;
	}
	CHECK(facingOk);
}

// 閉じた図形の体積（原点から各三角形への四面体の符号付き体積の合計）。向きが正しければ正になる
float SignedVolume(const MeshData& mesh) {
	float volume = 0.0f;
	for (size_t t = 0; t < mesh.indices.size() / 3; ++t) {
		Vector3 p0 = Pos(mesh.vertices[mesh.indices[t * 3]]);
		volume += Dot(p0, TriangleCross(mesh, t)) / 6.0f;
	}
	return volume;
}

struct Bounds {
	Vector3 min, max;
};

Bounds GetBounds(const MeshData& mesh) {
	Bounds b{ Pos(mesh.vertices[0]), Pos(mesh.vertices[0]) };
	for (const VertexData& v : mesh.vertices) {
		b.min = Min(b.min, Pos(v));
		b.max = Max(b.max, Pos(v));
	}
	return b;
}

} // namespace

TEST_CASE(PrimitiveMesh_AllTypesAreWellFormed) {
	for (PrimitiveType type : { PrimitiveType::kSphere, PrimitiveType::kBox, PrimitiveType::kPlane, PrimitiveType::kCylinder, PrimitiveType::kCone }) {
		CheckWellFormed(MakePrimitiveMeshData(type));
	}
}

TEST_CASE(PrimitiveMesh_Sphere) {
	MeshData mesh = MakeSphereMeshData();
	CHECK(mesh.vertices.size() == 16 * 16 * 4);
	CHECK(mesh.indices.size() == 16 * 16 * 6);
	CHECK_NEAR(SignedVolume(mesh), 4.0 / 3.0 * kPi, 0.2); // 半径1の球の体積（16分割の多角形近似なので、約3.5%小さくなる）
	Bounds b = GetBounds(mesh);
	CHECK_NEAR(b.max.y, 1.0, 1e-5);
	CHECK_NEAR(b.min.y, -1.0, 1e-5);
	CheckWellFormed(MakeSphereMeshData(0)); // 小さすぎる分割数は補正される
}

TEST_CASE(PrimitiveMesh_Box) {
	MeshData mesh = MakeBoxMeshData();
	CHECK(mesh.vertices.size() == 24);
	CHECK(mesh.indices.size() == 36);
	CHECK_NEAR(SignedVolume(mesh), 1.0, 1e-5);
	Bounds b = GetBounds(mesh);
	CHECK(NearlyEqual(b.min, Vector3{ -0.5f, -0.5f, -0.5f }));
	CHECK(NearlyEqual(b.max, Vector3{ 0.5f, 0.5f, 0.5f }));

	// 6面それぞれの法線が、軸方向を向いている
	int axisAligned = 0;
	for (const VertexData& v : mesh.vertices) {
		float maxComponent = (std::max)({ std::abs(v.normal.x), std::abs(v.normal.y), std::abs(v.normal.z) });
		axisAligned += NearlyEqual(maxComponent, 1.0f) ? 1 : 0;
	}
	CHECK(axisAligned == 24);

	MeshData big = MakeBoxMeshData(Vector3{ 2.0f, 4.0f, 6.0f });
	CHECK_NEAR(SignedVolume(big), 48.0, 1e-3);
	CheckWellFormed(big);
	Bounds bb = GetBounds(big);
	CHECK(NearlyEqual(bb.max, Vector3{ 1.0f, 2.0f, 3.0f }));
}

TEST_CASE(PrimitiveMesh_Plane) {
	MeshData mesh = MakePlaneMeshData(4.0f, 2.0f);
	CHECK(mesh.vertices.size() == 4);
	CHECK(mesh.indices.size() == 6);
	bool flatAndUp = true;
	for (const VertexData& v : mesh.vertices) {
		flatAndUp = flatAndUp && v.position.y == 0.0f && NearlyEqual(v.normal, Vector3{ 0.0f, 1.0f, 0.0f });
	}
	CHECK(flatAndUp);
	Bounds b = GetBounds(mesh);
	CHECK(NearlyEqual(b.max, Vector3{ 2.0f, 0.0f, 1.0f }));
	CHECK(NearlyEqual(b.min, Vector3{ -2.0f, 0.0f, -1.0f }));
	// 上から見える向き（+Y）に、時計回りになっている
	CHECK(TriangleCross(mesh, 0).y > 0.0f);
	CHECK(TriangleCross(mesh, 1).y > 0.0f);
}

TEST_CASE(PrimitiveMesh_Cylinder) {
	MeshData mesh = MakeCylinderMeshData(0.5f, 1.0f, 64);
	CHECK_NEAR(SignedVolume(mesh), kPi * 0.25 * 1.0, 0.01); // 円柱の体積 π r² h
	Bounds b = GetBounds(mesh);
	CHECK_NEAR(b.max.y, 0.5, 1e-5);
	CHECK_NEAR(b.min.y, -0.5, 1e-5);
	CHECK_NEAR(b.max.x, 0.5, 1e-5);
	CheckWellFormed(MakeCylinderMeshData(1.0f, 2.0f, 0)); // 小さすぎる分割数は補正される
	CHECK(SignedVolume(MakeCylinderMeshData(1.0f, 2.0f, 0)) > 0.0f);
}

TEST_CASE(PrimitiveMesh_Cone) {
	MeshData mesh = MakeConeMeshData(0.5f, 1.0f, 64);
	CHECK_NEAR(SignedVolume(mesh), kPi * 0.25 * 1.0 / 3.0, 0.01); // 円錐の体積 π r² h / 3
	Bounds b = GetBounds(mesh);
	CHECK_NEAR(b.max.y, 0.5, 1e-5); // 先端が +Y 側
	CHECK_NEAR(b.min.y, -0.5, 1e-5);

	// 側面の法線は上向きに傾いている（水平成分と、上向き成分の両方を持つ）
	bool sideTilted = true;
	for (const VertexData& v : mesh.vertices) {
		if (v.normal.y < 0.999f && v.normal.y > -0.999f) { // 底面（±1）以外 = 側面
			sideTilted = sideTilted && v.normal.y > 0.0f;
		}
	}
	CHECK(sideTilted);
	CheckWellFormed(MakeConeMeshData(1.0f, 3.0f, 3));
}

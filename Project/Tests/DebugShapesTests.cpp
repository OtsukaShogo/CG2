#include "TestFramework.h"
#include "Source/Engine/DebugTools/DebugShapes.h"
#include "Source/Engine/Math/MathUtil.h"
#include "Source/Engine/Math/TransformMatrix.h"

#include <algorithm>
#include <cmath>

using namespace Engine;
using namespace Engine::DebugShapes;

namespace {

Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }

float LineLength(const DebugLine& line) {
	return Length(Vector3{ line.end - line.start });
}

// 線が連続した閉じた輪になっているか（各線の終点が、次の線の始点と一致し、最後は最初へ戻る）
bool IsClosedLoop(const DebugLineList& lines, size_t begin, size_t end) {
	for (size_t i = begin; i < end; ++i) {
		size_t next = (i + 1 < end) ? i + 1 : begin;
		if (!NearlyEqual(lines[i].end, lines[next].start, 1e-5f)) {
			return false;
		}
	}
	return true;
}

} // namespace

TEST_CASE(DebugShapes_CircleOnPlane) {
	DebugLineList lines;
	AppendCircle(lines, V3(1, 2, 3), V3(1, 0, 0), V3(0, 0, 1), 2.0f, 16); // 水平（XZ）な円
	CHECK(lines.size() == 16);
	bool onCircle = true;
	for (const DebugLine& line : lines) {
		onCircle = onCircle && NearlyEqual(Length(Vector3{ line.start - V3(1, 2, 3) }), 2.0f, 1e-4f);
		onCircle = onCircle && NearlyEqual(line.start.y, 2.0f, 1e-5f); // 水平のまま
	}
	CHECK(onCircle);
	CHECK(IsClosedLoop(lines, 0, lines.size()));
}

TEST_CASE(DebugShapes_CircleFromNormal) {
	for (Vector3 normal : { V3(0, 1, 0), V3(1, 0, 0), V3(0, 0, 3), V3(1, 2, 3) }) {
		DebugLineList lines;
		AppendCircle(lines, V3(0, 0, 0), normal, 1.0f, 24);
		CHECK(lines.size() == 24);
		bool ok = true;
		for (const DebugLine& line : lines) {
			ok = ok && NearlyEqual(Length(line.start), 1.0f, 1e-4f);         // 半径1
			ok = ok && NearlyEqual(Dot(line.start, normal), 0.0f, 1e-4f);    // 法線に垂直な平面上
		}
		CHECK(ok);
	}
	DebugLineList none;
	AppendCircle(none, V3(0, 0, 0), V3(0, 0, 0), 1.0f, 24); // ゼロ法線は何も追加しない
	CHECK(none.empty());
}

TEST_CASE(DebugShapes_CircleSegmentsAreClamped) {
	DebugLineList lines;
	AppendCircle(lines, V3(0, 0, 0), V3(1, 0, 0), V3(0, 1, 0), 1.0f, 0);
	CHECK(lines.size() == 3);
}

TEST_CASE(DebugShapes_Sphere) {
	DebugLineList lines;
	AppendSphere(lines, V3(1, 1, 1), 2.0f, 12);
	CHECK(lines.size() == 36); // 3つの円
	bool onSphere = true;
	for (const DebugLine& line : lines) {
		onSphere = onSphere && NearlyEqual(Length(Vector3{ line.start - V3(1, 1, 1) }), 2.0f, 1e-4f);
	}
	CHECK(onSphere);
	CHECK(IsClosedLoop(lines, 0, 12));
	CHECK(IsClosedLoop(lines, 12, 24));
	CHECK(IsClosedLoop(lines, 24, 36));
}

TEST_CASE(DebugShapes_BoxAabb) {
	DebugLineList lines;
	AppendBox(lines, V3(-1, -2, -3), V3(1, 2, 3));
	CHECK(lines.size() == 12);
	// 辺の長さは、4本ずつ 2・4・6
	int x = 0, y = 0, z = 0;
	for (const DebugLine& line : lines) {
		float length = LineLength(line);
		x += NearlyEqual(length, 2.0f, 1e-5f) ? 1 : 0;
		y += NearlyEqual(length, 4.0f, 1e-5f) ? 1 : 0;
		z += NearlyEqual(length, 6.0f, 1e-5f) ? 1 : 0;
	}
	CHECK(x == 4);
	CHECK(y == 4);
	CHECK(z == 4);
	// すべての端点が、範囲内の角にある
	bool corners = true;
	for (const DebugLine& line : lines) {
		for (Vector3 p : { line.start, line.end }) {
			corners = corners && NearlyEqual(std::abs(p.x), 1.0f) && NearlyEqual(std::abs(p.y), 2.0f) && NearlyEqual(std::abs(p.z), 3.0f);
		}
	}
	CHECK(corners);
}

TEST_CASE(DebugShapes_BoxFromMatrixMatchesAabb) {
	// 単位行列なら、大きさ1・原点中心の立方体
	DebugLineList unit;
	AppendBox(unit, MakeIdentity4x4());
	CHECK(unit.size() == 12);
	bool allHalf = true;
	for (const DebugLine& line : unit) {
		allHalf = allHalf && NearlyEqual(std::abs(line.start.x), 0.5f) && NearlyEqual(std::abs(line.start.y), 0.5f) && NearlyEqual(std::abs(line.start.z), 0.5f);
	}
	CHECK(allHalf);

	// 回転のない行列は、AABBと同じ辺の集まりになる
	DebugLineList fromMatrix;
	AppendBox(fromMatrix, MakeAffineMatrix(V3(2, 4, 6), V3(0, 0, 0), V3(10, 0, 0)));
	DebugLineList fromAabb;
	AppendBox(fromAabb, V3(9, -2, -3), V3(11, 2, 3));
	CHECK(fromMatrix.size() == fromAabb.size());
	bool same = fromMatrix.size() == fromAabb.size();
	for (size_t i = 0; same && i < fromMatrix.size(); ++i) {
		same = NearlyEqual(fromMatrix[i].start, fromAabb[i].start, 1e-4f) && NearlyEqual(fromMatrix[i].end, fromAabb[i].end, 1e-4f);
	}
	CHECK(same);

	// 回転しても、辺の長さは変わらない
	DebugLineList rotated;
	AppendBox(rotated, MakeAffineMatrix(V3(2, 4, 6), V3(0.3f, 1.1f, -0.7f), V3(0, 0, 0)));
	float total = 0.0f;
	for (const DebugLine& line : rotated) {
		total += LineLength(line);
	}
	CHECK_NEAR(total, 4 * (2 + 4 + 6), 1e-3);
}

TEST_CASE(DebugShapes_RectXZ) {
	DebugLineList lines;
	AppendRect(lines, V3(5, 1, -2), Vector2{ 3.0f, 1.0f });
	CHECK(lines.size() == 4);
	CHECK(IsClosedLoop(lines, 0, 4));
	bool flat = true;
	float perimeter = 0.0f;
	for (const DebugLine& line : lines) {
		flat = flat && line.start.y == 1.0f && line.end.y == 1.0f;
		perimeter += LineLength(line);
	}
	CHECK(flat);
	CHECK_NEAR(perimeter, 2 * (6 + 2), 1e-5);
}

TEST_CASE(DebugShapes_Grid) {
	DebugLineList lines;
	AppendGrid(lines, V3(0, 0, 0), 2.0f, 1.0f);
	CHECK(lines.size() == 10); // -2..2 の5本 × 2方向
	bool inside = true;
	for (const DebugLine& line : lines) {
		inside = inside && std::abs(line.start.x) <= 2.0f + 1e-5f && std::abs(line.start.z) <= 2.0f + 1e-5f;
		inside = inside && std::abs(line.end.x) <= 2.0f + 1e-5f && std::abs(line.end.z) <= 2.0f + 1e-5f && line.start.y == 0.0f;
	}
	CHECK(inside);

	DebugLineList empty;
	AppendGrid(empty, V3(0, 0, 0), 2.0f, 0.0f);
	AppendGrid(empty, V3(0, 0, 0), 2.0f, -1.0f);
	AppendGrid(empty, V3(0, 0, 0), 0.0f, 1.0f);
	CHECK(empty.empty());

	DebugLineList partial;
	AppendGrid(partial, V3(0, 0, 0), 2.5f, 1.0f); // 端に届かない間隔
	CHECK(partial.size() == 10);
}

TEST_CASE(DebugShapes_Arrow) {
	DebugLineList lines;
	AppendArrow(lines, V3(0, 0, 0), V3(0, 0, 4), 1.0f);
	CHECK(lines.size() == 5); // 本体 + 矢じり4本
	CHECK_NEAR(LineLength(lines[0]), 4.0, 1e-5);
	// 矢じりの線は、すべて終点から始まり、手前（始点側）へ戻っている
	bool heads = true;
	for (size_t i = 1; i < lines.size(); ++i) {
		heads = heads && NearlyEqual(lines[i].start, V3(0, 0, 4)) && lines[i].end.z < 4.0f && lines[i].end.z > 2.5f;
	}
	CHECK(heads);

	DebugLineList zero;
	AppendArrow(zero, V3(1, 1, 1), V3(1, 1, 1), 1.0f); // 長さ0は何も追加しない
	CHECK(zero.empty());

	DebugLineList shortArrow;
	AppendArrow(shortArrow, V3(0, 0, 0), V3(0, 0, 0.2f), 5.0f); // 矢じりが長すぎても、全体の半分までに収まる
	bool clamped = shortArrow.size() == 5;
	for (size_t i = 1; clamped && i < shortArrow.size(); ++i) {
		clamped = shortArrow[i].end.z >= 0.1f - 1e-5f;
	}
	CHECK(clamped);
}

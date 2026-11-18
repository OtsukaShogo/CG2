#include "DebugShapes.h"
#include "../Math/MathUtil.h"
#include "../Math/TransformMatrix.h"
#include <algorithm>
#include <cmath>

namespace Engine {

namespace DebugShapes {

void AppendCircle(DebugLineList& out, const Vector3& center, const Vector3& u, const Vector3& v, float radius, uint32_t segments) {
	segments = (std::max)(segments, 3u);
	const float step = kTwoPi / static_cast<float>(segments);

	auto point = [&](uint32_t i) {
		float angle = step * static_cast<float>(i);
		float c = std::cos(angle) * radius;
		float s = std::sin(angle) * radius;
		return Vector3{ center.x + u.x * c + v.x * s, center.y + u.y * c + v.y * s, center.z + u.z * c + v.z * s };
	};

	Vector3 previous = point(0);
	for (uint32_t i = 1; i <= segments; ++i) {
		// 最後の点は、浮動小数点の誤差で最初の点とずれないよう、最初の点をそのまま使う
		Vector3 current = (i == segments) ? point(0) : point(i);
		out.push_back({ previous, current });
		previous = current;
	}
}

void AppendCircle(DebugLineList& out, const Vector3& center, const Vector3& normal, float radius, uint32_t segments) {
	Vector3 n = Normalize(normal);
	if (n == Vector3{ 0.0f, 0.0f, 0.0f }) {
		return; // 平面が定まらない
	}
	Vector3 u = Normalize(Perpendicular(n));
	Vector3 v = Cross(n, u);
	AppendCircle(out, center, u, v, radius, segments);
}

void AppendSphere(DebugLineList& out, const Vector3& center, float radius, uint32_t segments) {
	const Vector3 x = { 1.0f, 0.0f, 0.0f };
	const Vector3 y = { 0.0f, 1.0f, 0.0f };
	const Vector3 z = { 0.0f, 0.0f, 1.0f };
	AppendCircle(out, center, x, y, radius, segments); // XY平面
	AppendCircle(out, center, y, z, radius, segments); // YZ平面
	AppendCircle(out, center, z, x, radius, segments); // ZX平面（水平）
}

namespace {

// 8頂点から12本の辺を追加する。頂点の並びは、ビット0=X・ビット1=Y・ビット2=Z（0=最小側、1=最大側）
void AppendBoxEdges(DebugLineList& out, const Vector3 corners[8]) {
	for (int i = 0; i < 8; ++i) {
		for (int bit = 0; bit < 3; ++bit) {
			int j = i | (1 << bit);
			if (j != i) { // iの該当ビットが0のときだけ、そのビットを1にした頂点へ辺を引く（重複を避ける）
				out.push_back({ corners[i], corners[j] });
			}
		}
	}
}

} // namespace

void AppendBox(DebugLineList& out, const Vector3& min, const Vector3& max) {
	Vector3 corners[8];
	for (int i = 0; i < 8; ++i) {
		corners[i] = { (i & 1) ? max.x : min.x, (i & 2) ? max.y : min.y, (i & 4) ? max.z : min.z };
	}
	AppendBoxEdges(out, corners);
}

void AppendBox(DebugLineList& out, const Matrix4x4& world) {
	Vector3 corners[8];
	for (int i = 0; i < 8; ++i) {
		Vector3 local = { (i & 1) ? 0.5f : -0.5f, (i & 2) ? 0.5f : -0.5f, (i & 4) ? 0.5f : -0.5f };
		corners[i] = Transform(local, world);
	}
	AppendBoxEdges(out, corners);
}

void AppendRect(DebugLineList& out, const Vector3& center, const Vector2& halfSize) {
	Vector3 a = { center.x - halfSize.x, center.y, center.z - halfSize.y };
	Vector3 b = { center.x - halfSize.x, center.y, center.z + halfSize.y };
	Vector3 c = { center.x + halfSize.x, center.y, center.z + halfSize.y };
	Vector3 d = { center.x + halfSize.x, center.y, center.z - halfSize.y };
	out.push_back({ a, b });
	out.push_back({ b, c });
	out.push_back({ c, d });
	out.push_back({ d, a });
}

void AppendGrid(DebugLineList& out, const Vector3& center, float halfExtent, float step) {
	if (!(step > 0.0f) || !(halfExtent > 0.0f)) {
		return;
	}
	// 中心から両側へ、step 間隔で線を引く（端を超えない）。中心の線は1度だけ
	const int count = static_cast<int>(std::floor(halfExtent / step + 1e-4f));
	for (int i = -count; i <= count; ++i) {
		float offset = step * static_cast<float>(i);
		out.push_back({ { center.x + offset, center.y, center.z - halfExtent }, { center.x + offset, center.y, center.z + halfExtent } }); // Z方向の線
		out.push_back({ { center.x - halfExtent, center.y, center.z + offset }, { center.x + halfExtent, center.y, center.z + offset } }); // X方向の線
	}
}

void AppendArrow(DebugLineList& out, const Vector3& from, const Vector3& to, float headLength) {
	Vector3 delta = to - from;
	float length = Length(delta);
	if (!(length > 1e-6f)) {
		return;
	}
	out.push_back({ from, to });

	Vector3 direction = delta / length;
	float head = (std::min)(headLength, length * 0.5f);
	if (!(head > 0.0f)) {
		return;
	}
	Vector3 side1 = Normalize(Perpendicular(direction));
	Vector3 side2 = Cross(direction, side1);
	Vector3 back = to - direction * head;
	const float spread = head * 0.4f;
	out.push_back({ to, back + side1 * spread });
	out.push_back({ to, back - side1 * spread });
	out.push_back({ to, back + side2 * spread });
	out.push_back({ to, back - side2 * spread });
}

} // namespace DebugShapes

} // namespace Engine

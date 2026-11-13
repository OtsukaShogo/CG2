#include "TestFramework.h"
#include "Source/Engine/Math/MathUtil.h"
#include "Source/Engine/Math/Matrix4x4.h"
#include "Source/Engine/Math/Quaternion.h"
#include "Source/Engine/Math/TransformMatrix.h"
#include "Source/Engine/Math/Vector2.h"
#include "Source/Engine/Math/Vector3.h"

using namespace Engine;

namespace {

// 波括弧の初期化子だと、Vector2/Vector3/Quaternion のオーバーロードが曖昧になるので、型を明示して作る
Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
Vector2 V2(float x, float y) { return Vector2{ x, y }; }
Quaternion Q(float x, float y, float z, float w) { return Quaternion{ x, y, z, w }; }

bool MatNear(const Matrix4x4& a, const Matrix4x4& b, float eps = 1e-4f) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (!(std::abs(a.m[i][j] - b.m[i][j]) <= eps)) {
				return false;
			}
		}
	}
	return true;
}

} // namespace

TEST_CASE(Vector3_Operators) {
	Vector3 a = V3(1, 2, 3), b = V3(4, -5, 6);
	CHECK(a + b == V3(5, -3, 9));
	CHECK(a - b == V3(-3, 7, -3));
	CHECK(-a == V3(-1, -2, -3));
	CHECK(a * 2.0f == 2.0f * a);
	CHECK((a / 2.0f) == V3(0.5f, 1.0f, 1.5f));
	Vector3 c = a;
	c += b;
	c -= b;
	c *= 2.0f;
	c /= 2.0f;
	CHECK(c == a);
}

TEST_CASE(Vector3_Functions) {
	Vector3 a = V3(1, 2, 3), b = V3(4, -5, 6);
	CHECK(NearlyEqual(Dot(a, b), 12.0f));
	CHECK(Cross(V3(1, 0, 0), V3(0, 1, 0)) == V3(0, 0, 1));
	CHECK(NearlyEqual(Length(V3(3, 4, 0)), 5.0f));
	CHECK(NearlyEqual(Lerp(a, b, 0.5f), V3(2.5f, -1.5f, 4.5f)));
	CHECK(NearlyEqual(Project(V3(3, 4, 0), V3(1, 0, 0)), V3(3, 0, 0)));
	CHECK(Project(V3(1, 1, 1), V3(0, 0, 0)) == V3(0, 0, 0));
	CHECK(NearlyEqual(Reflect(V3(1, -1, 0), V3(0, 1, 0)), V3(1, 1, 0)));
	CHECK(Min(a, b) == V3(1, -5, 3));
	CHECK(Max(a, b) == V3(4, 2, 6));
}

TEST_CASE(Vector3_NormalizeIsZeroSafe) {
	CHECK(NearlyEqual(Length(Normalize(V3(0, 0, 7))), 1.0f));
	CHECK(Normalize(V3(0, 0, 0)) == V3(0, 0, 0));
	CHECK(Normalize(V3(1e-9f, 0, 0)) == V3(0, 0, 0)); // ほぼゼロでも安全
}

TEST_CASE(Vector3_PerpendicularAndClamp) {
	for (Vector3 v : { V3(1, 2, 3), V3(0, 0, 5), V3(0, 4, 0), V3(-2, 0, 0) }) {
		CHECK(NearlyEqual(Dot(v, Perpendicular(v)), 0.0f));
		CHECK(Length(Perpendicular(v)) > 0.0f);
	}
	CHECK(NearlyEqual(Length(ClampLength(V3(10, 0, 0), 3.0f)), 3.0f));
	CHECK(ClampLength(V3(1, 0, 0), 3.0f) == V3(1, 0, 0));
}

TEST_CASE(Vector2_Functions) {
	Vector2 p = V2(3, 4);
	CHECK(NearlyEqual(Length(p), 5.0f));
	CHECK(NearlyEqual(Length(Normalize(p)), 1.0f));
	CHECK(Normalize(V2(0, 0)) == V2(0, 0));
	CHECK(NearlyEqual(Cross(V2(1, 0), V2(0, 1)), 1.0f));
	CHECK(NearlyEqual(Rotate(V2(1, 0), kPi * 0.5f), V2(0, 1)));
	CHECK(NearlyEqual(ToAngle(V2(0, 1)), kPi * 0.5f));
	CHECK(NearlyEqual(FromAngle(0.0f), V2(1, 0)));
	CHECK(NearlyEqual(Perpendicular(V2(1, 0)), V2(0, 1)));
	CHECK(NearlyEqual(Distance(V2(0, 0), V2(3, 4)), 5.0f));
	CHECK(NearlyEqual(Length(ClampLength(V2(30, 40), 5.0f)), 5.0f));
	CHECK(NearlyEqual(Lerp(V2(0, 0), V2(2, 4), 0.5f), V2(1, 2)));
	CHECK((V2(1, 2) + V2(3, 4)) == V2(4, 6));
}

TEST_CASE(Matrix_InverseAndOperators) {
	Matrix4x4 m = MakeAffineMatrix({ 2, 3, 0.5f }, { 0.3f, -1.2f, 2.0f }, { 5, -6, 7 });
	CHECK(MatNear(m * Inverse(m), MakeIdentity4x4()));
	CHECK(MatNear(Inverse(m) * m, MakeIdentity4x4()));
	CHECK(MatNear(Inverse(MakeIdentity4x4()), MakeIdentity4x4()));
	Matrix4x4 singular = {};
	CHECK(MatNear(Inverse(singular), MakeIdentity4x4())); // 逆行列がない場合は単位行列（仕様）
	CHECK(MatNear(m + m - m, m));
	Matrix4x4 t = m;
	t *= MakeIdentity4x4();
	CHECK(MatNear(t, m));
}

TEST_CASE(Transform_PointAndNormal) {
	CHECK(NearlyEqual(Transform(V3(1, 0, 0), MakeTranslateMatrix(V3(1, 2, 3))), V3(2, 2, 3)));
	CHECK(NearlyEqual(TransformNormal(V3(1, 0, 0), MakeTranslateMatrix(V3(1, 2, 3))), V3(1, 0, 0))); // 平行移動の影響を受けない
	CHECK(NearlyEqual(TransformNormal(V3(1, 0, 0), MakeRotateZMatrix(kPi * 0.5f)), V3(0, 1, 0)));
}

TEST_CASE(Quaternion_MatchesRotationMatrices) {
	Quaternion qx = MakeRotateAxisAngleQuaternion(V3(1, 0, 0), 0.7f);
	CHECK(MatNear(MakeRotateMatrix(qx), MakeRotateXMatrix(0.7f)));
	CHECK(MatNear(MakeRotateMatrix(MakeRotateAxisAngleQuaternion(V3(0, 1, 0), -1.1f)), MakeRotateYMatrix(-1.1f)));
	CHECK(MatNear(MakeRotateMatrix(MakeRotateAxisAngleQuaternion(V3(0, 0, 2), 2.3f)), MakeRotateZMatrix(2.3f))); // 軸は正規化不要
	CHECK(NearlyEqual(MakeRotateMatrix(MakeRotateAxisAngleQuaternion(V3(0, 0, 0), 1.0f)).m[0][0], 1.0f));      // ゼロ軸は回転なし
	CHECK(NearlyEqual(RotateVector(V3(0, 1, 0), qx), Transform(V3(0, 1, 0), MakeRotateXMatrix(0.7f)), 1e-4f));
}

TEST_CASE(Quaternion_FromEulerMatchesAffineMatrix) {
	Vector3 euler = V3(0.4f, -0.9f, 1.3f);
	Matrix4x4 expected = MakeRotateXMatrix(euler.x) * MakeRotateYMatrix(euler.y) * MakeRotateZMatrix(euler.z);
	CHECK(MatNear(MakeRotateMatrix(MakeQuaternionFromEuler(euler)), expected));
	Quaternion q = MakeQuaternionFromEuler(euler);
	CHECK(MatNear(MakeAffineMatrixFromQuaternion({ 1, 2, 3 }, q, { 4, 5, 6 }), MakeAffineMatrix({ 1, 2, 3 }, euler, { 4, 5, 6 })));
}

TEST_CASE(Quaternion_NormalizeInverseCompose) {
	Quaternion q = Normalize(Q(1, 2, 3, 4));
	CHECK(NearlyEqual(Norm(q), 1.0f));
	Quaternion id = q * Inverse(q);
	CHECK_NEAR(id.x, 0, 1e-5);
	CHECK_NEAR(id.y, 0, 1e-5);
	CHECK_NEAR(id.z, 0, 1e-5);
	CHECK_NEAR(id.w, 1, 1e-5);
	CHECK(NearlyEqual(Norm(Normalize(Q(0, 0, 0, 0))), 1.0f)); // ゼロは回転なしになる

	// 先にX、次にY（Multiply(後, 先)）。行列は Rx * Ry
	Quaternion composed = Multiply(MakeRotateAxisAngleQuaternion(V3(0, 1, 0), 0.5f), MakeRotateAxisAngleQuaternion(V3(1, 0, 0), 0.8f));
	CHECK(MatNear(MakeRotateMatrix(composed), MakeRotateXMatrix(0.8f) * MakeRotateYMatrix(0.5f)));
}

TEST_CASE(Quaternion_Slerp) {
	Quaternion a0 = MakeRotateAxisAngleQuaternion(V3(0, 1, 0), 0.2f);
	Quaternion a1 = MakeRotateAxisAngleQuaternion(V3(0, 1, 0), 1.4f);
	CHECK(MatNear(MakeRotateMatrix(Slerp(a0, a1, 0.0f)), MakeRotateMatrix(a0)));
	CHECK(MatNear(MakeRotateMatrix(Slerp(a0, a1, 1.0f)), MakeRotateMatrix(a1)));
	CHECK(MatNear(MakeRotateMatrix(Slerp(a0, a1, 0.5f)), MakeRotateYMatrix(0.8f)));
	Quaternion neg = Q(-a1.x, -a1.y, -a1.z, -a1.w); // 同じ回転の反対符号でも、短い経路を通る
	CHECK(MatNear(MakeRotateMatrix(Slerp(a0, neg, 0.5f)), MakeRotateYMatrix(0.8f)));
	Quaternion nearQ = MakeRotateAxisAngleQuaternion(V3(0, 1, 0), 0.2f + 1e-4f);
	CHECK_NEAR(Norm(Slerp(a0, nearQ, 0.37f)), 1.0f, 1e-6); // ほぼ同じ回転でも長さが1のまま
	CHECK_NEAR(Norm(Slerp(a0, a0, 0.37f)), 1.0f, 1e-6);
}

TEST_CASE(Quaternion_DirectionToDirection) {
	Vector3 dirs[] = { V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(1, 2, 3), V3(-1, 0, 0), V3(0, 0, -1), V3(0, -2, 0) };
	for (Vector3 from : dirs) {
		for (Vector3 to : dirs) {
			Vector3 rotated = RotateVector(Normalize(from), DirectionToDirection(from, to));
			CHECK(NearlyEqual(rotated, Normalize(to), 1e-4f));
		}
	}
	CHECK(NearlyEqual(Norm(DirectionToDirection(V3(0, 0, 0), V3(1, 0, 0))), 1.0f));
}

TEST_CASE(MathUtil_Angles) {
	CHECK(NearlyEqual(ToRadians(180.0f), kPi));
	CHECK(NearlyEqual(ToDegrees(kPi), 180.0f));
	CHECK(NearlyEqual(WrapAngle(3.0f * kPi), kPi) || NearlyEqual(WrapAngle(3.0f * kPi), -kPi));
	CHECK(NearlyEqual(WrapAngle(-kPi * 1.5f), kPi * 0.5f));
	CHECK(NearlyEqual(DeltaAngle(ToRadians(350.0f), ToRadians(10.0f)), ToRadians(20.0f), 1e-4f));
	CHECK(NearlyEqual(DeltaAngle(ToRadians(10.0f), ToRadians(350.0f)), ToRadians(-20.0f), 1e-4f));
	CHECK(NearlyEqual(LerpAngle(ToRadians(350.0f), ToRadians(10.0f), 0.5f), ToRadians(360.0f), 1e-4f));
}

TEST_CASE(MathUtil_Scalars) {
	CHECK(MoveTowards(0.0f, 10.0f, 3.0f) == 3.0f);
	CHECK(MoveTowards(9.0f, 10.0f, 3.0f) == 10.0f);
	CHECK(MoveTowards(5.0f, 0.0f, 2.0f) == 3.0f);
	CHECK(Saturate(-1) == 0);
	CHECK(Saturate(2) == 1);
	CHECK(Saturate(0.3f) == 0.3f);
	CHECK(Clamp(5, 0, 3) == 3);
	CHECK(Clamp(-5, 0, 3) == 0);
	CHECK(Lerp(0.0f, 10.0f, 0.25f) == 2.5f);
}

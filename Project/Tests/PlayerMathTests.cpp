#include "TestFramework.h"
#include "Source/Application/Game/PlayerMath.h"
#include "Source/Engine/Math/MathUtil.h"

#include <cmath>

using namespace Engine;
using namespace Shooter;

namespace {

Vector2 V2(float x, float y) { return Vector2{ x, y }; }
Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
float Len(const Vector2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

MoveInput Keys(bool left, bool right, bool up, bool down) {
	MoveInput input;
	input.left = left;
	input.right = right;
	input.up = up;
	input.down = down;
	return input;
}

} // namespace

TEST_CASE(PlayerMath_KeyboardAxes) {
	CHECK(NearlyEqual(CombineMoveInput(Keys(false, true, false, false)), V2(1, 0)));
	CHECK(NearlyEqual(CombineMoveInput(Keys(true, false, false, false)), V2(-1, 0)));
	CHECK(NearlyEqual(CombineMoveInput(Keys(false, false, true, false)), V2(0, 1)));
	CHECK(NearlyEqual(CombineMoveInput(Keys(false, false, false, true)), V2(0, -1)));
	CHECK(NearlyEqual(CombineMoveInput(Keys(false, false, false, false)), V2(0, 0)));
}

TEST_CASE(PlayerMath_DiagonalIsNotFaster) {
	Vector2 diagonal = CombineMoveInput(Keys(false, true, true, false));
	CHECK_NEAR(Len(diagonal), 1.0, 1e-5); // 斜め押しでも、長さ1
	CHECK_NEAR(diagonal.x, diagonal.y, 1e-5);
	CHECK(diagonal.x > 0.0f);
}

TEST_CASE(PlayerMath_OpposingKeysCancel) {
	CHECK(NearlyEqual(CombineMoveInput(Keys(true, true, false, false)), V2(0, 0)));
	CHECK(NearlyEqual(CombineMoveInput(Keys(true, true, true, false)), V2(0, 1)));   // 左右は打ち消し、上だけ残る
	CHECK(NearlyEqual(CombineMoveInput(Keys(true, true, true, true)), V2(0, 0)));
}

TEST_CASE(PlayerMath_StickKeepsMagnitude) {
	MoveInput half;
	half.stick = V2(0.5f, 0.0f);
	CHECK(NearlyEqual(CombineMoveInput(half), V2(0.5f, 0.0f))); // 半分傾ければ、半分の速さ
	MoveInput full;
	full.stick = V2(0.0f, -1.0f);
	CHECK(NearlyEqual(CombineMoveInput(full), V2(0, -1)));
}

TEST_CASE(PlayerMath_KeyboardAndStickAreClamped) {
	MoveInput both = Keys(false, true, false, false);
	both.stick = V2(1.0f, 0.0f);
	CHECK_NEAR(Len(CombineMoveInput(both)), 1.0, 1e-5); // 足し合わせても、1を超えない
	MoveInput opposite = Keys(false, true, false, false);
	opposite.stick = V2(-1.0f, 0.0f);
	CHECK(NearlyEqual(CombineMoveInput(opposite), V2(0, 0)));
	MoveInput mixed = Keys(false, true, false, false);
	mixed.stick = V2(0.0f, 1.0f);
	CHECK_NEAR(Len(CombineMoveInput(mixed)), 1.0, 1e-5);
}

TEST_CASE(PlayerMath_ToGroundDirection) {
	CHECK(NearlyEqual(ToGroundDirection(V3(3, 5, 4)), V2(0.6f, 0.8f))); // 高さは無視して、水平成分だけを正規化
	CHECK(NearlyEqual(ToGroundDirection(V3(0, -1, 0)), V2(0, 0)));       // 真下を向いている: 水平成分なし
	CHECK(NearlyEqual(ToGroundDirection(V3(0, 0, 0)), V2(0, 0)));
}

TEST_CASE(PlayerMath_MoveInputToWorldFollowsCamera) {
	// カメラが標準の向き: 画面の右が+X、画面の上（奥）が+Z
	Vector2 right = V2(1, 0), up = V2(0, 1);
	CHECK(NearlyEqual(MoveInputToWorld(V2(0, 1), right, up), V3(0, 0, 1)));  // 奥
	CHECK(NearlyEqual(MoveInputToWorld(V2(1, 0), right, up), V3(1, 0, 0)));  // 右
	CHECK(NearlyEqual(MoveInputToWorld(V2(0.5f, 0.5f), right, up), V3(0.5f, 0, 0.5f)));

	// カメラを90度回して、画面の右が+Z・画面の上が-X: 「奥」へ進むと、ワールドでは-Xへ進む
	Vector2 rotatedRight = V2(0, 1), rotatedUp = V2(-1, 0);
	CHECK(NearlyEqual(MoveInputToWorld(V2(0, 1), rotatedRight, rotatedUp), V3(-1, 0, 0)));
	CHECK(NearlyEqual(MoveInputToWorld(V2(1, 0), rotatedRight, rotatedUp), V3(0, 0, 1)));

	// 入力の長さは保たれる（半分の傾きは、半分の速さ）
	Vector3 half = MoveInputToWorld(V2(0, 0.5f), rotatedRight, rotatedUp);
	CHECK_NEAR(Length(half), 0.5, 1e-5);
	CHECK_NEAR(half.y, 0.0, 1e-9);
}

TEST_CASE(PlayerMath_ApproachVelocity) {
	// 目標へ、1回に maxDelta ずつ近づく
	Vector3 v = ApproachVelocity(V3(0, 0, 0), V3(10, 0, 0), 3.0f);
	CHECK(NearlyEqual(v, V3(3, 0, 0)));
	v = ApproachVelocity(v, V3(10, 0, 0), 3.0f);
	CHECK(NearlyEqual(v, V3(6, 0, 0)));
	// 届く場合は、目標をちょうど返す（行き過ぎない）
	CHECK(NearlyEqual(ApproachVelocity(V3(9, 0, 0), V3(10, 0, 0), 3.0f), V3(10, 0, 0)));
	// 向きを変えるときも、直線で近づく
	Vector3 turned = ApproachVelocity(V3(5, 0, 0), V3(0, 0, 5), 2.0f);
	CHECK_NEAR(Length(Vector3{ turned - V3(0, 0, 5) }), Length(Vector3{ V3(5, 0, 0) - V3(0, 0, 5) }) - 2.0, 1e-4);
	// すでに目標にある / maxDelta が0
	CHECK(NearlyEqual(ApproachVelocity(V3(1, 0, 0), V3(1, 0, 0), 1.0f), V3(1, 0, 0)));
	CHECK(NearlyEqual(ApproachVelocity(V3(1, 0, 0), V3(5, 0, 0), 0.0f), V3(1, 0, 0)));
}

TEST_CASE(PlayerMath_YawFromDirection) {
	CHECK_NEAR(YawFromDirection(V3(0, 0, 1), 9.0f), 0.0, 1e-6);          // +Z が 0
	CHECK_NEAR(YawFromDirection(V3(1, 0, 0), 9.0f), kPi * 0.5, 1e-5);    // +X
	CHECK_NEAR(YawFromDirection(V3(-1, 0, 0), 9.0f), -kPi * 0.5, 1e-5);
	CHECK_NEAR(std::abs(YawFromDirection(V3(0, 0, -1), 9.0f)), kPi, 1e-5);
	CHECK_NEAR(YawFromDirection(V3(0, 5, 0), 1.25f), 1.25, 1e-6);        // 水平成分がない: そのまま
	CHECK_NEAR(YawFromDirection(V3(2, 7, 2), 9.0f), kPi * 0.25, 1e-5);   // 高さは無視
}

TEST_CASE(PlayerMath_RotateTowardsAngle) {
	CHECK_NEAR(RotateTowardsAngle(0.0f, 1.0f, 0.25f), 0.25, 1e-6);
	CHECK_NEAR(RotateTowardsAngle(0.0f, 1.0f, 5.0f), 1.0, 1e-6);         // 届く場合は、行き過ぎない
	CHECK_NEAR(RotateTowardsAngle(0.0f, -1.0f, 0.25f), -0.25, 1e-6);
	// 最短の向きで回る: 350度から10度へは、+側（20度）に回る
	float next = RotateTowardsAngle(ToRadians(350.0f) - 2.0f * kPi, ToRadians(10.0f), ToRadians(5.0f));
	CHECK_NEAR(next, ToRadians(-5.0f), 1e-4);
	// ほぼ反対向きでも、有限の値になる
	CHECK(std::isfinite(RotateTowardsAngle(0.0f, kPi, 0.1f)));
}

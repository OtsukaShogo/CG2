#include "PlayerMath.h"
#include "../../Engine/Math/MathUtil.h"
#include <algorithm>
#include <cmath>

using namespace Engine;

namespace Shooter {

Vector2 CombineMoveInput(const MoveInput& input) {
	// キーボード: 押されている向きを足して、長さ1にそろえる（左右・上下の同時押しは打ち消し合う）
	Vector2 keyboard = { 0.0f, 0.0f };
	keyboard.x += input.right ? 1.0f : 0.0f;
	keyboard.x -= input.left ? 1.0f : 0.0f;
	keyboard.y += input.up ? 1.0f : 0.0f;
	keyboard.y -= input.down ? 1.0f : 0.0f;
	const float keyboardLength = std::sqrt(keyboard.x * keyboard.x + keyboard.y * keyboard.y);
	if (keyboardLength > 0.0f) {
		keyboard = keyboard / keyboardLength;
	}

	// スティックと足し合わせて、長さ1までに収める
	Vector2 combined = keyboard + input.stick;
	const float length = std::sqrt(combined.x * combined.x + combined.y * combined.y);
	if (length > 1.0f) {
		combined = combined / length;
	}
	return combined;
}

Vector2 ToGroundDirection(const Vector3& v) {
	const float length = std::sqrt(v.x * v.x + v.z * v.z);
	if (length < 1e-6f) {
		return Vector2{ 0.0f, 0.0f };
	}
	return Vector2{ v.x / length, v.z / length };
}

Vector3 MoveInputToWorld(const Vector2& input, const Vector2& screenRight, const Vector2& screenUp) {
	return Vector3{
		screenRight.x * input.x + screenUp.x * input.y,
		0.0f,
		screenRight.y * input.x + screenUp.y * input.y,
	};
}

Vector3 ApproachVelocity(const Vector3& current, const Vector3& target, float maxDelta) {
	const Vector3 difference = target - current;
	const float distance = Length(difference);
	if (distance <= maxDelta || distance < 1e-6f) {
		return target;
	}
	return current + difference * (maxDelta / distance);
}

float YawFromDirection(const Vector3& direction, float fallback) {
	if (std::sqrt(direction.x * direction.x + direction.z * direction.z) < 1e-6f) {
		return fallback;
	}
	return std::atan2(direction.x, direction.z);
}

float RotateTowardsAngle(float current, float target, float maxStep) {
	const float delta = DeltaAngle(current, target);
	if (std::abs(delta) <= maxStep) {
		return WrapAngle(target);
	}
	return WrapAngle(current + (delta > 0.0f ? maxStep : -maxStep));
}

} // namespace Shooter

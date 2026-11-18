#include "ShootingMath.h"
#include <algorithm>
#include <cmath>

using namespace Engine;

namespace Shooter {

Vector3 AimDirectionFromPoint(const Vector3& from, const Vector3& aimPoint, const Vector3& fallback, float minDistance) {
	const float dx = aimPoint.x - from.x;
	const float dz = aimPoint.z - from.z;
	const float distance = std::sqrt(dx * dx + dz * dz);
	if (distance < minDistance || distance < 1e-6f) {
		return fallback;
	}
	return Vector3{ dx / distance, 0.0f, dz / distance };
}

bool IsStickActive(const Vector2& stick, float threshold) {
	return stick.x * stick.x + stick.y * stick.y > threshold * threshold;
}

AimSource ChooseAimSource(AimSource current, bool mouseActive, bool stickActive) {
	if (mouseActive && !stickActive) {
		return AimSource::kMouse;
	}
	if (stickActive && !mouseActive) {
		return AimSource::kGamepad;
	}
	return current;
}

int ConsumeShots(float& cooldown, float deltaTime, bool wantsToFire, float interval, int maxShots) {
	cooldown -= deltaTime;
	int shots = 0;
	while (wantsToFire && cooldown <= 0.0f && shots < maxShots) {
		++shots;
		cooldown += interval;
	}
	// 押していない間や、最大数に達したときは、余りを持ち越さない（次に押したとき、すぐ撃てて、まとめて出ない）
	cooldown = (std::max)(cooldown, 0.0f);
	return shots;
}

BulletStepResult AdvanceBullet(BulletState& bullet, float deltaTime, const AABB* walls, size_t wallCount, float radius) {
	BulletStepResult result;
	bullet.lifeRemaining -= deltaTime;

	const Vector3 next = bullet.position + bullet.velocity * deltaTime;
	const Segment3D path{ bullet.position, next };

	// 弾の大きさぶん壁を太らせて、点（弾の中心）として調べる。最も手前の壁を選ぶ
	float nearest = 2.0f;
	RayHit3D nearestHit;
	for (size_t i = 0; i < wallCount; ++i) {
		const AABB inflated{ walls[i].min - Vector3{ radius, radius, radius }, walls[i].max + Vector3{ radius, radius, radius } };
		const RayHit3D hit = Raycast(path, inflated);
		if (hit.hit && hit.t < nearest) {
			nearest = hit.t;
			nearestHit = hit;
		}
	}

	if (nearestHit.hit) {
		bullet.position = nearestHit.point;
		result.alive = false;
		result.hitWall = true;
		result.hitPoint = nearestHit.point;
		result.hitNormal = nearestHit.normal;
		return result;
	}

	bullet.position = next;
	result.alive = bullet.lifeRemaining > 0.0f;
	return result;
}

} // namespace Shooter

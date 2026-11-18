#include "CombatMath.h"
#include <algorithm>
#include <cmath>

using namespace Engine;

namespace Shooter {

Health MakeHealth(int maxHp) {
	Health health;
	health.hp = maxHp;
	health.maxHp = maxHp;
	return health;
}

bool IsDead(const Health& health) {
	return health.hp <= 0;
}

bool TakeDamage(Health& health, int amount, float invulnerableDuration) {
	if (amount <= 0 || IsDead(health) || health.invulnerableTime > 0.0f) {
		return false;
	}
	health.hp = (std::max)(0, health.hp - amount);
	health.invulnerableTime = (std::max)(0.0f, invulnerableDuration);
	return true;
}

void UpdateHealth(Health& health, float deltaTime) {
	health.invulnerableTime = (std::max)(0.0f, health.invulnerableTime - deltaTime);
}

float HealthRatio(const Health& health) {
	if (health.maxHp <= 0) {
		return 0.0f;
	}
	return (std::clamp)(static_cast<float>(health.hp) / static_cast<float>(health.maxHp), 0.0f, 1.0f);
}

bool IsBlinkVisible(float invulnerableTime, float period) {
	if (invulnerableTime <= 0.0f || period <= 0.0f) {
		return true;
	}
	// 無敵の残り時間を、period ごとに区切る。奇数番目だけ表示する（残りの最後の区間で、表示に戻る）
	const int step = static_cast<int>(std::floor(invulnerableTime / period));
	return (step % 2) == 0;
}

bool IsInContact(const Vector3& a, const Vector3& b, float radiusSum, float margin) {
	const float dx = a.x - b.x;
	const float dz = a.z - b.z;
	const float reach = radiusSum + margin;
	return dx * dx + dz * dz <= reach * reach;
}

bool BulletHitsCapsule(const Segment3D& path, float bulletRadius, const Capsule& target, float* outT) {
	Capsule inflated = target;
	inflated.radius += bulletRadius;
	const RayHit3D hit = Raycast(path, inflated);
	if (hit.hit && outT) {
		*outT = hit.t;
	}
	return hit.hit;
}

} // namespace Shooter

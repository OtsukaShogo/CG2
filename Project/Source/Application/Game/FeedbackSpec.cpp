#include "FeedbackSpec.h"
#include "../../Engine/Math/MathUtil.h"

using namespace Engine;

namespace Shooter {

FeedbackSpec GetFeedbackSpec(FeedbackType type) {
	switch (type) {
	case FeedbackType::kBulletFired:   return { SeId::kShot, EffectId::kMuzzleFlash };
	case FeedbackType::kBulletTrail:   return { SeId::kNone, EffectId::kBulletTrail };
	case FeedbackType::kBulletHitWall: return { SeId::kNone, EffectId::kBulletImpact };
	case FeedbackType::kEnemyHit:      return { SeId::kEnemyHit, EffectId::kEnemyHit };
	case FeedbackType::kEnemyKilled:   return { SeId::kEnemyKill, EffectId::kEnemyKill };
	case FeedbackType::kPlayerHit:     return { SeId::kPlayerHit, EffectId::kPlayerHit };
	case FeedbackType::kPlayerDied:    return { SeId::kPlayerHit, EffectId::kPlayerDeath };
	}
	return {};
}

EffectBurst MakeBurst(EffectId id, const Vector3& direction) {
	EffectBurst b;
	b.count = 0;
	switch (id) {
	case EffectId::kNone:
		break;

	case EffectId::kMuzzleFlash: // 前へ、短く鋭く飛ぶ、黄色い閃光
		b.count = 14;
		b.speedMin = 3.0f; b.speedMax = 10.0f;
		b.lifetimeMin = 0.08f; b.lifetimeMax = 0.18f;
		b.sizeMin = 0.26f; b.sizeMax = 0.46f;
		b.endSizeRatio = 0.2f;
		b.startColor = { 1.0f, 0.95f, 0.6f, 1.0f };
		b.endColor = { 1.0f, 0.6f, 0.1f, 0.0f };
		b.direction = direction;
		b.spreadAngle = ToRadians(35.0f);
		break;

	case EffectId::kBulletTrail: // その場に残る、小さな光（飛んだあとに、尾を引く）
		b.count = 10;
		b.speedMin = 0.0f; b.speedMax = 1.2f;
		b.positionJitter = { 0.08f, 0.08f, 0.08f };
		b.lifetimeMin = 0.18f; b.lifetimeMax = 0.3f;
		b.sizeMin = 0.28f; b.sizeMax = 0.36f;
		b.endSizeRatio = 0.0f;
		b.startColor = { 1.0f, 0.85f, 0.3f, 0.8f };
		b.endColor = { 1.0f, 0.4f, 0.1f, 0.0f };
		break;

	case EffectId::kBulletImpact: // 壁の面から、跳ね返るように、落ちながら散る火花
		b.count = 22;
		b.speedMin = 3.0f; b.speedMax = 9.0f;
		b.lifetimeMin = 0.15f; b.lifetimeMax = 0.32f;
		b.sizeMin = 0.2f; b.sizeMax = 0.36f;
		b.endSizeRatio = 0.0f;
		b.startColor = { 1.0f, 0.9f, 0.5f, 1.0f };
		b.endColor = { 1.0f, 0.5f, 0.1f, 0.0f };
		b.direction = direction;
		b.spreadAngle = ToRadians(70.0f);
		b.acceleration = { 0.0f, -7.0f, 0.0f };
		break;

	case EffectId::kEnemyHit: // 弾が来た側へ、跳ね返る白い火花
		b.count = 20;
		b.speedMin = 3.0f; b.speedMax = 7.0f;
		b.lifetimeMin = 0.15f; b.lifetimeMax = 0.3f;
		b.sizeMin = 0.22f; b.sizeMax = 0.42f;
		b.endSizeRatio = 0.0f;
		b.startColor = { 1.0f, 1.0f, 0.85f, 1.0f };
		b.endColor = { 1.0f, 0.45f, 0.15f, 0.0f };
		b.direction = Vector3{ -direction.x, -direction.y, -direction.z };
		b.spreadAngle = ToRadians(55.0f);
		b.drag = 3.0f;
		break;

	case EffectId::kEnemyKill: // 全方向へ、大きく散る、橙から赤の爆発
		b.count = 40;
		b.speedMin = 2.0f; b.speedMax = 8.0f;
		b.lifetimeMin = 0.35f; b.lifetimeMax = 0.7f;
		b.sizeMin = 0.4f; b.sizeMax = 0.8f;
		b.endSizeRatio = 0.25f;
		b.startColor = { 1.0f, 0.6f, 0.2f, 1.0f };
		b.endColor = { 0.8f, 0.1f, 0.05f, 0.0f };
		b.positionJitter = { 0.15f, 0.2f, 0.15f };
		b.acceleration = { 0.0f, -3.0f, 0.0f };
		b.drag = 2.0f;
		break;

	case EffectId::kPlayerHit: // 自機の周りに、赤く広がる
		b.count = 36;
		b.speedMin = 3.0f; b.speedMax = 8.0f;
		b.lifetimeMin = 0.3f; b.lifetimeMax = 0.55f;
		b.sizeMin = 0.5f; b.sizeMax = 0.95f;
		b.endSizeRatio = 0.2f;
		b.startColor = { 1.0f, 0.55f, 0.45f, 1.0f };
		b.endColor = { 1.0f, 0.15f, 0.1f, 0.0f };
		b.drag = 2.5f;
		break;

	case EffectId::kPlayerDeath: // 自機の色（青白）の、大きな爆発
		b.count = 70;
		b.speedMin = 3.0f; b.speedMax = 11.0f;
		b.lifetimeMin = 0.6f; b.lifetimeMax = 1.1f;
		b.sizeMin = 0.45f; b.sizeMax = 0.9f;
		b.endSizeRatio = 0.2f;
		b.startColor = { 0.85f, 0.95f, 1.0f, 1.0f };
		b.endColor = { 0.3f, 0.5f, 1.0f, 0.0f };
		b.positionJitter = { 0.25f, 0.4f, 0.25f };
		b.acceleration = { 0.0f, -2.0f, 0.0f };
		b.drag = 1.8f;
		break;
	}
	return b;
}

} // namespace Shooter

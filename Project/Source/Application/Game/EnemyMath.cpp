#include "EnemyMath.h"
#include "ShootingMath.h"
#include <algorithm>
#include <cmath>

using namespace Engine;

namespace Shooter {

Vector3 DirectionToTarget(const Vector3& from, const Vector3& to) {
	const float dx = to.x - from.x;
	const float dz = to.z - from.z;
	const float distance = std::sqrt(dx * dx + dz * dz);
	if (distance < 1e-6f) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}
	return Vector3{ dx / distance, 0.0f, dz / distance };
}

int ConsumeSpawns(float& timer, float deltaTime, float interval, int aliveCount, int maxAlive, int maxPerFrame) {
	// 連射の間隔の管理と同じ仕組み（余りを持ち越し、処理落ちのあとの大量発生を防ぐ）。
	// 上限に達している間は、「出したい」を false にして、待ち時間だけを進める（0で止まり、空いた瞬間に出る）
	const bool canSpawn = aliveCount < maxAlive;
	const int room = (std::max)(0, maxAlive - aliveCount);
	const int shots = ConsumeShots(timer, deltaTime, canSpawn, interval, (std::min)(maxPerFrame, room));
	return shots;
}

Vector3 PerimeterPoint(float t, float halfWidth, float halfDepth, float margin) {
	const float hw = (std::max)(0.0f, halfWidth - margin);
	const float hd = (std::max)(0.0f, halfDepth - margin);
	const float width = hw * 2.0f;
	const float depth = hd * 2.0f;
	const float perimeter = 2.0f * (width + depth);

	float distance = (t - std::floor(t)) * perimeter; // 0〜1 に収める
	// 左手前の角 (-hw, -hd) から、時計回り（手前の辺 → 左の辺 → 奥の辺 → 右の辺）
	if (distance < width) {
		return Vector3{ -hw + distance, 0.0f, -hd };
	}
	distance -= width;
	if (distance < depth) {
		return Vector3{ hw, 0.0f, -hd + distance };
	}
	distance -= depth;
	if (distance < width) {
		return Vector3{ hw - distance, 0.0f, hd };
	}
	distance -= width;
	return Vector3{ -hw, 0.0f, hd - distance };
}

Vector3 ChooseSpawnPoint(std::mt19937& random, float halfWidth, float halfDepth, float margin, const Vector3& playerPosition, float minDistance) {
	std::uniform_real_distribution<float> uniform(0.0f, 1.0f);

	constexpr int kMaxAttempts = 24;
	for (int i = 0; i < kMaxAttempts; ++i) {
		const Vector3 point = PerimeterPoint(uniform(random), halfWidth, halfDepth, margin);
		const float dx = point.x - playerPosition.x;
		const float dz = point.z - playerPosition.z;
		if (dx * dx + dz * dz >= minDistance * minDistance) {
			return point;
		}
	}

	// 見つからなかった: 4つの角のうち、自機から最も遠いもの
	Vector3 farthest = PerimeterPoint(0.0f, halfWidth, halfDepth, margin);
	float farthestDistance = -1.0f;
	for (int corner = 0; corner < 4; ++corner) {
		const float width = (std::max)(0.0f, halfWidth - margin) * 2.0f;
		const float depth = (std::max)(0.0f, halfDepth - margin) * 2.0f;
		const float perimeter = 2.0f * (width + depth);
		const float cornerDistance[4] = { 0.0f, width, width + depth, 2.0f * width + depth };
		const Vector3 point = PerimeterPoint(perimeter > 0.0f ? cornerDistance[corner] / perimeter : 0.0f, halfWidth, halfDepth, margin);
		const float dx = point.x - playerPosition.x;
		const float dz = point.z - playerPosition.z;
		const float d = dx * dx + dz * dz;
		if (d > farthestDistance) {
			farthestDistance = d;
			farthest = point;
		}
	}
	return farthest;
}

Vector3 SeparationPush(const Vector3& a, const Vector3& b, float radiusSum) {
	const float dx = a.x - b.x;
	const float dz = a.z - b.z;
	const float distanceSquared = dx * dx + dz * dz;
	if (distanceSquared >= radiusSum * radiusSum) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}
	const float distance = std::sqrt(distanceSquared);
	if (distance < 1e-6f) {
		return Vector3{ radiusSum, 0.0f, 0.0f }; // 中心が一致: 向きが決まらないので、+X へ
	}
	const float overlap = radiusSum - distance;
	return Vector3{ dx / distance * overlap, 0.0f, dz / distance * overlap };
}

float SpawnInScale(float age, float duration) {
	if (duration <= 0.0f) {
		return 1.0f;
	}
	const float t = (std::clamp)(age / duration, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t); // 滑らかに（始まりと終わりがゆっくり）
}

} // namespace Shooter

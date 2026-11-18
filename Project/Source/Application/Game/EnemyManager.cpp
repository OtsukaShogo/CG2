#include "EnemyManager.h"
#include "Arena.h"
#include "EnemyMath.h"
#include "PlayerMath.h"
#include "ShooterConfig.h"
#include "../../Engine/DebugTools/DebugDraw.h"
#include <cmath>

using namespace Engine;

namespace Shooter {

namespace {

// 敵の体の、基準の大きさ（箱の基準サイズは一辺1）
constexpr float kBodyWidth = Config::kEnemyRadius * 2.0f * 0.9f;

} // namespace

void EnemyManager::Initialize(AssetFactory* assetFactory, size_t capacity) {
	auto box = assetFactory->CreatePrimitiveModelData(PrimitiveType::kBox);
	pool_ = std::make_unique<ObjectPool<Enemy>>(capacity, [&](size_t) {
		Enemy enemy;
		enemy.body = assetFactory->CreateModelInstance(box);
		enemy.body->SetColor(Vector4{ 0.9f, 0.25f, 0.25f, 1.0f });
		enemy.eye = assetFactory->CreateModelInstance(box);
		enemy.eye->SetColor(Vector4{ 1.0f, 0.95f, 0.5f, 1.0f });
		enemy.eye->GetMaterial().SetEnableLighting(false);
		return enemy;
	});
	scratch_.reserve(capacity);
}

bool EnemyManager::SpawnAt(const Vector3& position) {
	Enemy* enemy = pool_->Acquire();
	if (!enemy) {
		return false;
	}
	// プールのオブジェクトには、前回の状態が残っているので、すべて設定し直す
	enemy->position = Vector3{ position.x, 0.0f, position.z };
	enemy->velocity = Vector3{ 0.0f, 0.0f, 0.0f };
	enemy->yaw = 0.0f;
	enemy->age = 0.0f;
	SyncModels(*enemy);
	return true;
}

void EnemyManager::SyncModels(Enemy& enemy) {
	// 出現演出: 小さい状態から、滑らかに大きくなる
	const float scale = SpawnInScale(enemy.age, Config::kEnemySpawnInTime);
	const float height = Config::kEnemyHeight * scale;

	enemy.body->GetScale() = Vector3{ kBodyWidth * scale, height, kBodyWidth * scale };
	enemy.body->GetTranslate() = Vector3{ enemy.position.x, height * 0.5f, enemy.position.z };
	enemy.body->GetRotate() = Vector3{ 0.0f, enemy.yaw, 0.0f };

	const Vector3 forward = Vector3{ std::sin(enemy.yaw), 0.0f, std::cos(enemy.yaw) };
	enemy.eye->GetScale() = Vector3{ 0.5f * scale, 0.2f * scale, 0.15f * scale };
	enemy.eye->GetTranslate() = enemy.position + forward * (kBodyWidth * 0.5f * scale) + Vector3{ 0.0f, height * 0.75f, 0.0f };
	enemy.eye->GetRotate() = Vector3{ 0.0f, enemy.yaw, 0.0f };
}

void EnemyManager::Update(float deltaTime, const Vector3& playerPosition, float playerRadius, const Arena& arena) {
	const Vector2 halfExtent = arena.GetHalfExtent();

	// 1) 出現: アリーナの外周の、自機から離れた位置に
	if (spawningEnabled_) {
		const int spawns = ConsumeSpawns(spawnTimer_, deltaTime, Config::kEnemySpawnInterval,
			static_cast<int>(pool_->GetActiveCount()), Config::kEnemyMaxAlive);
		for (int i = 0; i < spawns; ++i) {
			SpawnAt(ChooseSpawnPoint(random_, halfExtent.x, halfExtent.y, Config::kEnemySpawnMargin, playerPosition, Config::kEnemySpawnMinDistance));
		}
	}

	// 2) 追跡: 自機へ向かって、加速しながら進む（出現演出の間は、動かない）
	scratch_.clear();
	for (Enemy& enemy : *pool_) {
		enemy.age += deltaTime;
		if (enemy.age < Config::kEnemySpawnInTime) {
			continue;
		}
		const Vector3 target = DirectionToTarget(enemy.position, playerPosition) * Config::kEnemySpeed;
		enemy.velocity = ApproachVelocity(enemy.velocity, target, Config::kEnemyAcceleration * deltaTime);
		enemy.position += enemy.velocity * deltaTime;

		if (Length(enemy.velocity) > 0.3f) {
			enemy.yaw = RotateTowardsAngle(enemy.yaw, YawFromDirection(enemy.velocity, enemy.yaw), Config::kEnemyTurnSpeed * deltaTime);
		}
		scratch_.push_back(&enemy);
	}

	// 3) 押し合い: 敵どうしは、お互いに半分ずつ。自機とは、敵だけが押し戻される。群がったときも重なりが残らないよう、4回繰り返す
	const float pairDistance = Config::kEnemyRadius * 2.0f;
	for (int iteration = 0; iteration < 4; ++iteration) {
		for (size_t i = 0; i < scratch_.size(); ++i) {
			for (size_t j = i + 1; j < scratch_.size(); ++j) {
				const Vector3 push = SeparationPush(scratch_[i]->position, scratch_[j]->position, pairDistance);
				if (push.x != 0.0f || push.z != 0.0f) {
					scratch_[i]->position += push * 0.5f;
					scratch_[j]->position -= push * 0.5f;
				}
			}
			scratch_[i]->position += SeparationPush(scratch_[i]->position, playerPosition, Config::kEnemyRadius + playerRadius);
		}
	}

	// 4) 壁との押し戻し（押し合いで、壁にめり込んでいないように、最後に行う）
	for (Enemy* enemy : scratch_) {
		Capsule capsule = MakeUprightCapsule(enemy->position, Config::kEnemyHeight, Config::kEnemyRadius);
		enemy->position += arena.ResolveCapsule(capsule);
		enemy->position.y = 0.0f;
	}

	for (Enemy& enemy : *pool_) {
		SyncModels(enemy);
	}
}

void EnemyManager::Draw(const Matrix4x4& viewProjection) {
	for (Enemy& enemy : *pool_) {
		enemy.body->Draw(viewProjection);
		enemy.eye->Draw(viewProjection);
	}
}

void EnemyManager::DrawDebug(DebugDraw& debugDraw) const {
	for (const Enemy& enemy : *pool_) {
		// 出現演出の間は、判定がないので、薄い色で示す
		const bool spawning = enemy.age < Config::kEnemySpawnInTime;
		debugDraw.Draw(MakeUprightCapsule(enemy.position, Config::kEnemyHeight, Config::kEnemyRadius),
			spawning ? DebugColor::kGray : DebugColor::kRed, 12);
	}
}

void EnemyManager::Clear() {
	if (pool_) {
		pool_->ReleaseAll();
	}
	spawnTimer_ = 0.0f;
}

} // namespace Shooter

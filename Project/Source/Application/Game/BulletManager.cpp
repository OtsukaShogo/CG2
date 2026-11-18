#include "BulletManager.h"
#include "EnemyManager.h"
#include "ShooterConfig.h"
#include "../../Engine/DebugTools/DebugDraw.h"

using namespace Engine;

namespace Shooter {

void BulletManager::Initialize(AssetFactory* assetFactory, size_t capacity) {
	auto sphere = assetFactory->CreatePrimitiveModelData(PrimitiveType::kSphere);
	pool_ = std::make_unique<ObjectPool<Bullet>>(capacity, [&](size_t) {
		Bullet bullet;
		bullet.model = assetFactory->CreateModelInstance(sphere);
		// 球の基準サイズは半径1なので、弾の半径に合わせて縮める。光を受けず、常に明るく見えるようにする
		const float scale = Config::kBulletRadius;
		bullet.model->GetScale() = Vector3{ scale, scale, scale };
		bullet.model->SetColor(Vector4{ 1.0f, 0.9f, 0.3f, 1.0f });
		bullet.model->GetMaterial().SetEnableLighting(false);
		return bullet;
	});
}

void BulletManager::Fire(const Vector3& position, const Vector3& direction) {
	Bullet* bullet = pool_->AcquireOrRecycleOldest();
	if (!bullet) {
		return; // 容量0
	}
	// プールのオブジェクトには、前回の状態が残っているので、すべて設定し直す
	bullet->state.position = position;
	bullet->state.velocity = direction * Config::kBulletSpeed;
	bullet->state.lifeRemaining = Config::kBulletLifetime;
	bullet->model->GetTranslate() = position;
}

void BulletManager::Update(float deltaTime, const std::array<AABB, 4>& walls, EnemyManager& enemies) {
	// 走査しながら返却してよい
	for (Bullet& bullet : *pool_) {
		const Vector3 previous = bullet.state.position;
		const BulletStepResult result = AdvanceBullet(bullet.state, deltaTime, walls.data(), walls.size(), Config::kBulletRadius);

		// 敵への命中: 前の位置から今の位置（壁に当たった場合は、壁の手前）までの道のりで調べる
		Vector3 hitPoint;
		if (enemies.TryHitByBullet(Segment3D{ previous, bullet.state.position }, Config::kBulletRadius, Config::kBulletDamage, &hitPoint)) {
			bullet.state.position = hitPoint;
			bullet.model->GetTranslate() = hitPoint;
			pool_->Release(&bullet);
			continue;
		}

		bullet.model->GetTranslate() = bullet.state.position;
		if (!result.alive) {
			pool_->Release(&bullet);
		}
	}
}

void BulletManager::Draw(const Matrix4x4& viewProjection) {
	for (Bullet& bullet : *pool_) {
		bullet.model->Draw(viewProjection);
	}
}

void BulletManager::DrawDebug(DebugDraw& debugDraw) const {
	for (const Bullet& bullet : *pool_) {
		debugDraw.Draw(Sphere{ bullet.state.position, Config::kBulletRadius }, DebugColor::kYellow, 8);
	}
}

void BulletManager::Clear() {
	if (pool_) {
		pool_->ReleaseAll();
	}
}

} // namespace Shooter

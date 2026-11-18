#pragma once
#include <array>
#include <cstddef>
#include <memory>
#include "ShootingMath.h"
#include "../../Engine/Containers/ObjectPool.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Math/Matrix4x4.h"

namespace Engine { class DebugDraw; }

namespace Shooter {

class EnemyManager;

/// <summary>
/// 自機の弾をまとめて管理する。弾はプールで使い回すので、撃つたびにモデルを作り直さない
/// </summary>
class BulletManager {
public:
	/// <summary>
	/// 弾のプール（容量ぶんのモデルを、最初に1度だけ作る）を用意する
	/// </summary>
	void Initialize(Engine::AssetFactory* assetFactory, size_t capacity);

	/// <summary>
	/// 弾を1発撃つ。プールが満杯のときは、最も古い弾を消して、再利用する
	/// </summary>
	/// <param name="position">弾が出る位置</param>
	/// <param name="direction">撃つ向き（長さ1）</param>
	void Fire(const Engine::Vector3& position, const Engine::Vector3& direction);

	/// <summary>
	/// すべての弾を進める。敵・壁に当たった弾、寿命の尽きた弾は、プールへ返却する。
	/// 敵への命中は、前フレームの位置から今の位置までの線分で調べるので、速い弾が、敵をすり抜けない
	/// （壁に当たった場合は、壁の手前までの線分なので、壁の向こうの敵には当たらない）
	/// </summary>
	void Update(float deltaTime, const std::array<Engine::AABB, 4>& walls, EnemyManager& enemies);

	/// <summary>
	/// 飛んでいる弾を描画する
	/// </summary>
	void Draw(const Engine::Matrix4x4& viewProjection);

	/// <summary>
	/// 弾の判定（球）を、デバッグ表示に追加する
	/// </summary>
	void DrawDebug(Engine::DebugDraw& debugDraw) const;

	/// <summary>
	/// すべての弾を消す（シーンのやり直し時など）
	/// </summary>
	void Clear();

	/// <summary>
	/// 飛んでいる弾の数
	/// </summary>
	[[nodiscard]] size_t GetActiveCount() const { return pool_ ? pool_->GetActiveCount() : 0; }

private:
	struct Bullet {
		std::unique_ptr<Engine::Model> model;
		BulletState state;
	};

	std::unique_ptr<Engine::ObjectPool<Bullet>> pool_;
};

} // namespace Shooter

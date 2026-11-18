#pragma once
#include <cstddef>
#include <memory>
#include <random>
#include <vector>
#include "../../Engine/Containers/ObjectPool.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Math/Matrix4x4.h"
#include "../../Engine/Math/Vector3.h"

namespace Engine { class DebugDraw; }

namespace Shooter {

class Arena;

/// <summary>
/// 敵をまとめて管理する。敵はプールで使い回すので、出現のたびにモデルを作り直さない。
/// 出現（アリーナの外周から、自機から離れた位置に）、自機への追跡、敵どうし・自機・壁との押し合いを受け持つ
/// </summary>
class EnemyManager {
public:
	/// <summary>
	/// 敵のプール（容量ぶんのモデルを、最初に1度だけ作る）を用意する
	/// </summary>
	void Initialize(Engine::AssetFactory* assetFactory, size_t capacity);

	/// <summary>
	/// 1フレーム進める（出現 → 追跡 → 押し合い → 壁との押し戻し）
	/// </summary>
	/// <param name="playerPosition">自機の位置（足元）</param>
	/// <param name="playerRadius">自機の半径（敵が自機にめり込まないように）</param>
	void Update(float deltaTime, const Engine::Vector3& playerPosition, float playerRadius, const Arena& arena);

	/// <summary>
	/// 指定の位置に、敵を1体出す（出現演出から始まる）。プールが満杯なら、何もしない
	/// </summary>
	/// <returns>出せた場合は true</returns>
	bool SpawnAt(const Engine::Vector3& position);

	/// <summary>
	/// 敵を描画する
	/// </summary>
	void Draw(const Engine::Matrix4x4& viewProjection);

	/// <summary>
	/// 敵の判定（カプセル）を、デバッグ表示に追加する
	/// </summary>
	void DrawDebug(Engine::DebugDraw& debugDraw) const;

	/// <summary>
	/// すべての敵を消す
	/// </summary>
	void Clear();

	/// <summary>
	/// 自動の出現を、オン・オフする（デバッグ用）
	/// </summary>
	void SetSpawningEnabled(bool enabled) { spawningEnabled_ = enabled; }
	[[nodiscard]] bool IsSpawningEnabled() const { return spawningEnabled_; }

	/// <summary>
	/// 生きている敵の数
	/// </summary>
	[[nodiscard]] size_t GetActiveCount() const { return pool_ ? pool_->GetActiveCount() : 0; }

private:
	struct Enemy {
		std::unique_ptr<Engine::Model> body;
		std::unique_ptr<Engine::Model> eye; // 向きが分かるように、前に付ける小さな箱
		Engine::Vector3 position = { 0.0f, 0.0f, 0.0f };
		Engine::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
		float yaw = 0.0f;
		float age = 0.0f; // 出現してからの時間
	};

	/// <summary>
	/// モデルの位置・向き・大きさを、現在の状態に合わせる
	/// </summary>
	static void SyncModels(Enemy& enemy);

private:
	std::unique_ptr<Engine::ObjectPool<Enemy>> pool_;
	std::vector<Enemy*> scratch_; // 押し合いの計算に使う、出現演出の終わった敵の一覧（毎フレーム使い回す）
	std::mt19937 random_{ std::random_device{}() };
	float spawnTimer_ = 0.0f;
	bool spawningEnabled_ = true;
};

} // namespace Shooter

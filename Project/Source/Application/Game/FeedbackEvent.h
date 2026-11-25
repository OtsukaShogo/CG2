#pragma once
#include <cstddef>
#include <vector>
#include "../../Engine/Math/Vector3.h"

namespace Shooter {

/// <summary>
/// ゲーム中に起きた、演出（パーティクル）や効果音のきっかけになる出来事
/// </summary>
enum class FeedbackType {
	kBulletFired,   //!< 弾を撃った（位置 = 銃口、向き = 撃つ向き）
	kBulletTrail,   //!< 飛んでいる弾の軌跡（位置 = 弾の位置、向き = 進む向き）
	kBulletHitWall, //!< 弾が壁に当たった（位置 = 当たった点、向き = 壁の面の向き）
	kEnemyHit,      //!< 弾が敵に当たった（位置 = 当たった点、向き = 弾の進む向き）
	kEnemyKilled,   //!< 敵を倒した（位置 = 敵の胸の位置）
	kPlayerHit,     //!< 自機がダメージを受けた（位置 = 自機の胸の位置）
	kPlayerDied,    //!< 自機が倒れた（位置 = 自機の胸の位置）
};

/// <summary>
/// 出来事1件分
/// </summary>
struct FeedbackEvent {
	FeedbackType type = FeedbackType::kBulletFired;
	Engine::Vector3 position = { 0.0f, 0.0f, 0.0f };
	Engine::Vector3 direction = { 0.0f, 0.0f, 0.0f }; //!< 出来事ごとの意味は、FeedbackType のコメントのとおり（使わないなら 0）
};

/// <summary>
/// 1フレームのあいだに起きた出来事を、貯めておく入れ物。
/// 弾・敵・自機の側は、出来事を入れるだけにして、演出と効果音は、まとめて、あとで処理する（それぞれの処理を、直接つながない）。
/// 1フレームに入れられる数には上限があり、超えた分は捨てる（演出が、処理を重くしないように）
/// </summary>
class FeedbackQueue {
public:
	static constexpr size_t kMaxEventsPerFrame = 1024;

	/// <summary>
	/// 出来事を入れる。上限を超えている場合は、何もしない
	/// </summary>
	/// <returns>入れられたか</returns>
	bool Push(FeedbackType type, const Engine::Vector3& position, const Engine::Vector3& direction = { 0.0f, 0.0f, 0.0f });

	[[nodiscard]] const std::vector<FeedbackEvent>& GetEvents() const { return events_; }

	/// <summary>
	/// 溜まった出来事を、すべて消す（処理したあとに呼ぶ）
	/// </summary>
	void Clear() { events_.clear(); }

private:
	std::vector<FeedbackEvent> events_;
};

} // namespace Shooter

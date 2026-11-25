#pragma once
#include "AudioTable.h"
#include "FeedbackEvent.h"
#include "../../Engine/3D/ParticleEffectMath.h"

namespace Shooter {

/// <summary>
/// 演出（パーティクル）の種類
/// </summary>
enum class EffectId {
	kNone,
	kMuzzleFlash,  //!< 銃口の閃光
	kBulletTrail,  //!< 弾の軌跡
	kBulletImpact, //!< 壁に当たった火花
	kEnemyHit,     //!< 敵に当たった火花
	kEnemyKill,    //!< 敵を倒した爆発
	kPlayerHit,    //!< 自機の被弾
	kPlayerDeath,  //!< 自機が倒れた爆発
};

/// <summary>
/// 出来事1件に対して、鳴らす効果音と、出す演出
/// </summary>
struct FeedbackSpec {
	SeId se = SeId::kNone;
	EffectId effect = EffectId::kNone;
};

/// <summary>
/// 出来事の種類から、効果音と演出を決める（対応表）
/// </summary>
[[nodiscard]] FeedbackSpec GetFeedbackSpec(FeedbackType type);

/// <summary>
/// 演出の設定（粒の数・速さ・色・寿命など）を作る。
/// direction は、向きを持つ演出（銃口の閃光・壁の火花・敵の火花）で使い、飛び散る中心の向きになる。
/// 向きを持たない演出（爆発・被弾）では、使わない。kNone なら、粒の数が 0 の設定を返す
/// </summary>
[[nodiscard]] Engine::EffectBurst MakeBurst(EffectId id, const Engine::Vector3& direction);

} // namespace Shooter

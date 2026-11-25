#include "TestFramework.h"
#include "Source/Application/Game/AudioTable.h"
#include "Source/Application/Game/FeedbackEvent.h"
#include "Source/Application/Game/FeedbackSpec.h"
#include "Source/Engine/Math/MathUtil.h"

#include <cmath>
#include <cstring>
#include <set>
#include <string>

using namespace Engine;
using namespace Shooter;

namespace {

Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }

const FeedbackType kAllTypes[] = {
	FeedbackType::kBulletFired, FeedbackType::kBulletTrail, FeedbackType::kBulletHitWall, FeedbackType::kEnemyHit,
	FeedbackType::kEnemyKilled, FeedbackType::kPlayerHit, FeedbackType::kPlayerDied,
};

const EffectId kAllEffects[] = {
	EffectId::kMuzzleFlash, EffectId::kBulletTrail, EffectId::kBulletImpact, EffectId::kEnemyHit,
	EffectId::kEnemyKill, EffectId::kPlayerHit, EffectId::kPlayerDeath,
};

} // namespace

// ---- 出来事の入れ物 ----

TEST_CASE(FeedbackQueue_PushAndClear) {
	FeedbackQueue queue;
	CHECK(queue.GetEvents().empty());
	CHECK(queue.Push(FeedbackType::kEnemyHit, V3(1, 2, 3), V3(0, 0, 1)));
	CHECK(queue.Push(FeedbackType::kEnemyKilled, V3(4, 5, 6)));
	CHECK(queue.GetEvents().size() == 2);
	CHECK(queue.GetEvents()[0].type == FeedbackType::kEnemyHit);
	CHECK(NearlyEqual(queue.GetEvents()[0].position, V3(1, 2, 3)));
	CHECK(NearlyEqual(queue.GetEvents()[0].direction, V3(0, 0, 1)));
	CHECK(NearlyEqual(queue.GetEvents()[1].direction, V3(0, 0, 0))); // 向きを指定しなければ、0
	queue.Clear();
	CHECK(queue.GetEvents().empty());
}

TEST_CASE(FeedbackQueue_DropsBeyondLimit) {
	FeedbackQueue queue;
	int accepted = 0;
	for (size_t i = 0; i < FeedbackQueue::kMaxEventsPerFrame + 50; ++i) {
		accepted += queue.Push(FeedbackType::kBulletTrail, V3(0, 0, 0)) ? 1 : 0;
	}
	CHECK(static_cast<size_t>(accepted) == FeedbackQueue::kMaxEventsPerFrame); // 上限を超えた分は捨てる
	CHECK(queue.GetEvents().size() == FeedbackQueue::kMaxEventsPerFrame);
	queue.Clear();
	CHECK(queue.Push(FeedbackType::kBulletTrail, V3(0, 0, 0))); // 空にすれば、また入る
}

// ---- 出来事 → 効果音と演出 ----

TEST_CASE(FeedbackSpec_RequestedEventsHaveSounds) {
	// ご要望の効果音: 弾を撃つ・被弾（自機と敵）・撃破
	CHECK(GetFeedbackSpec(FeedbackType::kBulletFired).se == SeId::kShot);
	CHECK(GetFeedbackSpec(FeedbackType::kPlayerHit).se == SeId::kPlayerHit);
	CHECK(GetFeedbackSpec(FeedbackType::kEnemyHit).se == SeId::kEnemyHit);
	CHECK(GetFeedbackSpec(FeedbackType::kEnemyKilled).se == SeId::kEnemyKill);
}

TEST_CASE(FeedbackSpec_RequestedEventsHaveEffects) {
	// ご要望のパーティクル: 弾（銃口・軌跡・着弾）・敵の被弾・自機の被弾
	CHECK(GetFeedbackSpec(FeedbackType::kBulletFired).effect == EffectId::kMuzzleFlash);
	CHECK(GetFeedbackSpec(FeedbackType::kBulletTrail).effect == EffectId::kBulletTrail);
	CHECK(GetFeedbackSpec(FeedbackType::kBulletHitWall).effect == EffectId::kBulletImpact);
	CHECK(GetFeedbackSpec(FeedbackType::kEnemyHit).effect == EffectId::kEnemyHit);
	CHECK(GetFeedbackSpec(FeedbackType::kPlayerHit).effect == EffectId::kPlayerHit);
	CHECK(GetFeedbackSpec(FeedbackType::kEnemyKilled).effect == EffectId::kEnemyKill);
}

TEST_CASE(FeedbackSpec_EveryEventDoesSomething) {
	for (FeedbackType type : kAllTypes) {
		const FeedbackSpec spec = GetFeedbackSpec(type);
		CHECK(spec.se != SeId::kNone || spec.effect != EffectId::kNone); // 何も起きない出来事は、ない
	}
	// 毎フレーム出る軌跡と、壁の着弾は、鳴らさない（うるさくならないように）
	CHECK(GetFeedbackSpec(FeedbackType::kBulletTrail).se == SeId::kNone);
	CHECK(GetFeedbackSpec(FeedbackType::kBulletHitWall).se == SeId::kNone);
}

// ---- 演出の設定 ----

TEST_CASE(FeedbackSpec_BurstsAreValid) {
	for (EffectId id : kAllEffects) {
		const EffectBurst b = MakeBurst(id, V3(0, 0, 1));
		CHECK(b.count > 0);
		CHECK(b.speedMin >= 0.0f && b.speedMin <= b.speedMax);
		CHECK(b.lifetimeMin > 0.0f && b.lifetimeMin <= b.lifetimeMax);
		CHECK(b.lifetimeMax < 2.0f);                   // 長く残りすぎない
		CHECK(b.sizeMin > 0.0f && b.sizeMin <= b.sizeMax);
		CHECK(b.endSizeRatio >= 0.0f && b.endSizeRatio <= 1.0f);
		CHECK(b.spreadAngle >= 0.0f && b.spreadAngle <= kPi + 1e-5f);
		CHECK(b.startColor.w > 0.0f);                  // 最初は見える
		CHECK(b.endColor.w <= b.startColor.w);         // 終わりに向けて、消えていく
	}
}

TEST_CASE(FeedbackSpec_NoneProducesNothing) {
	CHECK(MakeBurst(EffectId::kNone, V3(0, 0, 1)).count == 0);
}

TEST_CASE(FeedbackSpec_DirectionalBurstsFollowDirection) {
	// 銃口の閃光: 撃つ向きへ。壁の火花: 壁の面の向きへ
	const Vector3 forward = V3(1, 0, 0);
	const EffectBurst muzzle = MakeBurst(EffectId::kMuzzleFlash, forward);
	CHECK(NearlyEqual(muzzle.direction, forward));
	CHECK(muzzle.spreadAngle < kPi * 0.5f);            // 前方に絞られている
	const EffectBurst impact = MakeBurst(EffectId::kBulletImpact, V3(0, 0, -1));
	CHECK(NearlyEqual(impact.direction, V3(0, 0, -1)));
	CHECK(impact.spreadAngle < kPi * 0.5f);
	// 敵の火花: 弾の進む向きと、反対（弾が来た側へ跳ね返る）
	const EffectBurst enemyHit = MakeBurst(EffectId::kEnemyHit, V3(0, 0, 1));
	CHECK(NearlyEqual(enemyHit.direction, V3(0, 0, -1)));
}

TEST_CASE(FeedbackSpec_NonDirectionalBurstsSpreadEverywhere) {
	for (EffectId id : { EffectId::kEnemyKill, EffectId::kPlayerHit, EffectId::kPlayerDeath }) {
		const EffectBurst b = MakeBurst(id, V3(0, 0, 1));
		CHECK(b.spreadAngle >= kPi - 1e-4f);           // 全方向
	}
	// 向きの指定は、結果に影響しない
	CHECK(NearlyEqual(MakeBurst(EffectId::kEnemyKill, V3(0, 0, 1)).direction, MakeBurst(EffectId::kEnemyKill, V3(5, 0, 0)).direction));
}

TEST_CASE(FeedbackSpec_ImportantEventsAreBigger) {
	// 撃破や自機の死は、通常の被弾より、派手
	CHECK(MakeBurst(EffectId::kEnemyKill, V3(0, 0, 1)).count > MakeBurst(EffectId::kEnemyHit, V3(0, 0, 1)).count);
	CHECK(MakeBurst(EffectId::kPlayerDeath, V3(0, 0, 1)).count > MakeBurst(EffectId::kPlayerHit, V3(0, 0, 1)).count);
}

// ---- 音の表 ----

TEST_CASE(AudioTable_AllSoundsHavePathsAndValidVolumes) {
	for (int i = static_cast<int>(SeId::kShot); i < static_cast<int>(SeId::kCount); ++i) {
		const SeId id = static_cast<SeId>(i);
		CHECK(std::strlen(GetSePath(id)) > 0);
		CHECK(GetSeVolume(id) > 0.0f && GetSeVolume(id) <= 1.0f);
		CHECK(GetSeMinInterval(id) >= 0.0f);
	}
	for (int i = 0; i < static_cast<int>(BgmId::kCount); ++i) {
		const BgmId id = static_cast<BgmId>(i);
		CHECK(std::strlen(GetBgmPath(id)) > 0);
		CHECK(GetBgmVolume(id) > 0.0f && GetBgmVolume(id) <= 1.0f);
	}
}

TEST_CASE(AudioTable_NoneAndOutOfRangeAreSafe) {
	CHECK(std::strlen(GetSePath(SeId::kNone)) == 0);
	CHECK(GetSeVolume(SeId::kNone) == 0.0f);
	CHECK(std::strlen(GetSePath(SeId::kCount)) == 0);        // 範囲外でも、壊れない
	CHECK(std::strlen(GetBgmPath(BgmId::kCount)) == 0);
	CHECK(GetBgmVolume(BgmId::kCount) == 0.0f);
}

TEST_CASE(AudioTable_PlaceholderFilesAreAsRequested) {
	// 仮置き: 効果音はすべて shot.mp3、BGM はすべて game.mp3
	for (int i = static_cast<int>(SeId::kShot); i < static_cast<int>(SeId::kCount); ++i) {
		CHECK(std::string(GetSePath(static_cast<SeId>(i))) == "Resource/Audio/SE/shot.mp3");
	}
	for (int i = 0; i < static_cast<int>(BgmId::kCount); ++i) {
		CHECK(std::string(GetBgmPath(static_cast<BgmId>(i))) == "Resource/Audio/BGM/game.mp3");
	}
}

TEST_CASE(AudioTable_EveryRequestedSoundExists) {
	// 被弾（自機・敵）・弾を撃つ・撃破・決定
	std::set<int> ids;
	for (SeId id : { SeId::kShot, SeId::kPlayerHit, SeId::kEnemyHit, SeId::kEnemyKill, SeId::kConfirm }) {
		ids.insert(static_cast<int>(id));
		CHECK(std::strlen(GetSePath(id)) > 0);
	}
	CHECK(ids.size() == 5);
}

// ---- 鳴らす間隔の間引き ----

TEST_CASE(SoundThrottle_FirstPlayIsAllowedThenBlocked) {
	SoundThrottle throttle;
	CHECK(throttle.TryPlay(SeId::kShot));    // 最初は鳴らせる
	CHECK(!throttle.TryPlay(SeId::kShot));   // 直後は、間引く
	CHECK(throttle.TryPlay(SeId::kEnemyKill)); // 別の種類は、独立
}

TEST_CASE(SoundThrottle_AllowsAgainAfterInterval) {
	SoundThrottle throttle;
	CHECK(throttle.TryPlay(SeId::kShot));
	throttle.Update(GetSeMinInterval(SeId::kShot) * 0.5f);
	CHECK(!throttle.TryPlay(SeId::kShot));   // 間隔の途中
	throttle.Update(GetSeMinInterval(SeId::kShot) * 0.6f);
	CHECK(throttle.TryPlay(SeId::kShot));    // 間隔が過ぎた
}

TEST_CASE(SoundThrottle_LimitsRapidEventsToIntervalRate) {
	// 60fps で、毎フレーム鳴らそうとしても、間隔ぶんしか鳴らない（10秒で、約 10 / 間隔 回）
	SoundThrottle throttle;
	int played = 0;
	for (int i = 0; i < 600; ++i) {
		throttle.Update(1.0f / 60.0f);
		played += throttle.TryPlay(SeId::kShot) ? 1 : 0;
	}
	const float expected = 10.0f / GetSeMinInterval(SeId::kShot);
	CHECK(played >= expected * 0.8f && played <= expected + 1.0f);
	CHECK(played < 600);
}

TEST_CASE(SoundThrottle_NoneAndResetAndOutOfRange) {
	SoundThrottle throttle;
	CHECK(!throttle.TryPlay(SeId::kNone));    // 鳴らさない指定は、常に false
	CHECK(!throttle.TryPlay(SeId::kCount));
	CHECK(throttle.TryPlay(SeId::kConfirm));
	throttle.Reset();
	CHECK(throttle.TryPlay(SeId::kConfirm));  // リセットすれば、すぐ鳴らせる
	throttle.Update(100.0f);                  // 長い時間が過ぎても、おかしくならない
	CHECK(throttle.TryPlay(SeId::kConfirm));
}

// ---- BGM の切り替え ----

TEST_CASE(BgmTransition_SameTrackKeepsPlaying) {
	CHECK(DecideBgmTransition("a.mp3", true, "a.mp3") == BgmTransition::kKeep);   // 同じ曲が再生中: 続ける
}

TEST_CASE(BgmTransition_DifferentTrackSwitches) {
	CHECK(DecideBgmTransition("a.mp3", true, "b.mp3") == BgmTransition::kSwitch);
}

TEST_CASE(BgmTransition_NotPlayingOrNothingSwitches) {
	CHECK(DecideBgmTransition("a.mp3", false, "a.mp3") == BgmTransition::kSwitch); // 同じ曲でも、止まっていれば、始める
	CHECK(DecideBgmTransition("", false, "a.mp3") == BgmTransition::kSwitch);
	CHECK(DecideBgmTransition("", true, "a.mp3") == BgmTransition::kSwitch);
}

TEST_CASE(BgmTransition_PlaceholderBgmsContinueAcrossScenes) {
	// いまの仮置き（すべて同じ曲）では、タイトル → ゲーム → リザルトで、曲が最初に戻らない
	const std::string title = GetBgmPath(BgmId::kTitle);
	CHECK(DecideBgmTransition(title, true, GetBgmPath(BgmId::kGame)) == BgmTransition::kKeep);
	CHECK(DecideBgmTransition(GetBgmPath(BgmId::kGame), true, GetBgmPath(BgmId::kResult)) == BgmTransition::kKeep);
}

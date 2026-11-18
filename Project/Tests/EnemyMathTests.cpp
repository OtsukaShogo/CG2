#include "TestFramework.h"
#include "Source/Application/Game/EnemyMath.h"

#include <cmath>
#include <set>

using namespace Engine;
using namespace Shooter;

namespace {

Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
float Dist(const Vector3& a, const Vector3& b) { return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z)); }

} // namespace

TEST_CASE(EnemyMath_DirectionToTarget) {
	CHECK(NearlyEqual(DirectionToTarget(V3(0, 0, 0), V3(5, 0, 0)), V3(1, 0, 0)));
	CHECK(NearlyEqual(DirectionToTarget(V3(1, 0, 1), V3(4, 0, 5)), V3(0.6f, 0, 0.8f)));
	CHECK(NearlyEqual(DirectionToTarget(V3(0, 3, 0), V3(0, -7, -2)), V3(0, 0, -1))); // 高さは無視
	CHECK(NearlyEqual(DirectionToTarget(V3(2, 0, 2), V3(2, 9, 2)), V3(0, 0, 0)));    // 水平に距離がない
	CHECK(NearlyEqual(DirectionToTarget(V3(0, 0, 0), V3(0, 0, 0)), V3(0, 0, 0)));
}

TEST_CASE(EnemyMath_SpawnTimingFirstIsImmediate) {
	float timer = 0.0f;
	CHECK(ConsumeSpawns(timer, 0.016f, 1.0f, 0, 10) == 1); // 最初の1体は、すぐ
	CHECK(ConsumeSpawns(timer, 0.016f, 1.0f, 1, 10) == 0);
}

TEST_CASE(EnemyMath_SpawnRateIsFrameRateIndependent) {
	// 10秒・間隔1秒 → 約10体（生きている数は増えないものとして数える）
	for (float dt : { 1.0f / 144.0f, 1.0f / 60.0f, 1.0f / 30.0f, 1.0f / 10.0f }) {
		float timer = 0.0f;
		int total = 0;
		const int frames = static_cast<int>(10.0f / dt + 0.5f);
		for (int i = 0; i < frames; ++i) {
			total += ConsumeSpawns(timer, dt, 1.0f, 0, 1000);
		}
		CHECK(total >= 10 && total <= 11);
	}
}

TEST_CASE(EnemyMath_SpawnRespectsMaxAlive) {
	float timer = 0.0f;
	CHECK(ConsumeSpawns(timer, 0.016f, 1.0f, 5, 5) == 0);   // 上限: 出さない
	CHECK(ConsumeSpawns(timer, 0.016f, 1.0f, 9, 5) == 0);   // 上限を超えていても、出さない
	// 上限に達している間も、待ち時間は進んでいる（空いた瞬間に出る）
	float waiting = 0.5f;
	for (int i = 0; i < 100; ++i) (void)ConsumeSpawns(waiting, 0.016f, 1.0f, 5, 5);
	CHECK(waiting == 0.0f);
	CHECK(ConsumeSpawns(waiting, 0.016f, 1.0f, 4, 5) == 1);
	// 空きが1つなら、1体だけ（間隔が短くても、上限を超えない）
	float fast = 0.0f;
	CHECK(ConsumeSpawns(fast, 5.0f, 0.1f, 4, 5, 10) == 1);
}

TEST_CASE(EnemyMath_SpawnBurstIsCapped) {
	float timer = 0.0f;
	CHECK(ConsumeSpawns(timer, 10.0f, 0.5f, 0, 100) == 2);          // 長い処理落ちのあとも、既定では1フレーム2体まで
	float timer2 = 0.0f;
	CHECK(ConsumeSpawns(timer2, 10.0f, 0.5f, 0, 100, 5) == 5);      // 上限は指定できる
}

TEST_CASE(EnemyMath_PerimeterPointStaysOnInnerRectangle) {
	const float hw = 20.0f, hd = 14.0f, margin = 1.5f;
	const float iw = hw - margin, id = hd - margin;
	bool onEdge = true;
	for (int i = 0; i < 1000; ++i) {
		const Vector3 p = PerimeterPoint(i / 1000.0f, hw, hd, margin);
		const bool onVertical = std::abs(std::abs(p.x) - iw) < 1e-4f && std::abs(p.z) <= id + 1e-4f;
		const bool onHorizontal = std::abs(std::abs(p.z) - id) < 1e-4f && std::abs(p.x) <= iw + 1e-4f;
		onEdge = onEdge && (onVertical || onHorizontal) && p.y == 0.0f;
	}
	CHECK(onEdge);

	// 角と、1周して戻ること
	CHECK(NearlyEqual(PerimeterPoint(0.0f, hw, hd, margin), V3(-iw, 0, -id)));
	CHECK(NearlyEqual(PerimeterPoint(1.0f, hw, hd, margin), V3(-iw, 0, -id)));
	CHECK(NearlyEqual(PerimeterPoint(0.25f, hw, hd, margin), PerimeterPoint(0.25f, hw, hd, margin)));
	// 範囲外の t（負・1超）も、1周の中に収まる
	const Vector3 wrapped = PerimeterPoint(1.3f, hw, hd, margin);
	CHECK(NearlyEqual(wrapped, PerimeterPoint(0.3f, hw, hd, margin), 1e-3f));
	const Vector3 negative = PerimeterPoint(-0.2f, hw, hd, margin);
	CHECK(NearlyEqual(negative, PerimeterPoint(0.8f, hw, hd, margin), 1e-3f));
	// 4辺すべてを通る
	std::set<int> sides;
	for (int i = 0; i < 400; ++i) {
		const Vector3 p = PerimeterPoint(i / 400.0f, hw, hd, margin);
		if (std::abs(p.z + id) < 1e-4f) sides.insert(0);
		if (std::abs(p.x - iw) < 1e-4f) sides.insert(1);
		if (std::abs(p.z - id) < 1e-4f) sides.insert(2);
		if (std::abs(p.x + iw) < 1e-4f) sides.insert(3);
	}
	CHECK(sides.size() == 4);
}

TEST_CASE(EnemyMath_ChooseSpawnPointKeepsDistanceFromPlayer) {
	std::mt19937 random(12345);
	const float hw = 20.0f, hd = 14.0f, margin = 1.5f, minDistance = 14.0f;
	int tooClose = 0;
	std::set<int> sides;
	for (int n = 0; n < 2000; ++n) {
		// 自機は、アリーナのあちこちにいる
		const Vector3 player = V3(static_cast<float>(n % 37) - 18.0f, 0, static_cast<float>(n % 23) - 11.0f);
		const Vector3 p = ChooseSpawnPoint(random, hw, hd, margin, player, minDistance);
		tooClose += Dist(p, player) < minDistance;
		if (std::abs(p.z + (hd - margin)) < 1e-3f) sides.insert(0);
		if (std::abs(p.x - (hw - margin)) < 1e-3f) sides.insert(1);
		if (std::abs(p.z - (hd - margin)) < 1e-3f) sides.insert(2);
		if (std::abs(p.x + (hw - margin)) < 1e-3f) sides.insert(3);
		// 必ず、外周の上
		const bool onVertical = std::abs(std::abs(p.x) - (hw - margin)) < 1e-3f;
		const bool onHorizontal = std::abs(std::abs(p.z) - (hd - margin)) < 1e-3f;
		tooClose += !(onVertical || onHorizontal);
	}
	CHECK(tooClose == 0);
	CHECK(sides.size() == 4); // 偏らず、4辺から出る
}

TEST_CASE(EnemyMath_ChooseSpawnPointFallsBackToFarthestCorner) {
	std::mt19937 random(1);
	// 自機が左手前にいて、どこも近すぎる（minDistance が巨大）: 最も遠い角（右奥）
	const Vector3 p = ChooseSpawnPoint(random, 20.0f, 14.0f, 1.5f, V3(-18, 0, -12), 1000.0f);
	CHECK(NearlyEqual(p, V3(18.5f, 0, 12.5f)));
	// 自機が右奥なら、左手前
	const Vector3 q = ChooseSpawnPoint(random, 20.0f, 14.0f, 1.5f, V3(18, 0, 12), 1000.0f);
	CHECK(NearlyEqual(q, V3(-18.5f, 0, -12.5f)));
	// 同じ種なら、同じ結果（再現できる）
	std::mt19937 a(99), b(99);
	CHECK(NearlyEqual(ChooseSpawnPoint(a, 20, 14, 1.5f, V3(0, 0, 0), 10), ChooseSpawnPoint(b, 20, 14, 1.5f, V3(0, 0, 0), 10)));
}

TEST_CASE(EnemyMath_SeparationPush) {
	// 重ならない / ちょうど接する: 動かさない
	CHECK(NearlyEqual(SeparationPush(V3(0, 0, 0), V3(2, 0, 0), 1.0f), V3(0, 0, 0)));
	CHECK(NearlyEqual(SeparationPush(V3(0, 0, 0), V3(1, 0, 0), 1.0f), V3(0, 0, 0)));
	// 重なる: bから離れる向きへ、重なりの分だけ
	CHECK(NearlyEqual(SeparationPush(V3(0, 0, 0), V3(0.6f, 0, 0), 1.0f), V3(-0.4f, 0, 0), 1e-5f)); // 重なり = 半径の和(1.0) - 距離(0.6) = 0.4
}

TEST_CASE(EnemyMath_SeparationPushResolvesOverlap) {
	// 押したあとの距離が、半径の和と等しくなる
	const Vector3 a = V3(1, 0, 2), b = V3(1.5f, 0, 2.3f);
	const Vector3 push = SeparationPush(a, b, 1.0f);
	const Vector3 moved = V3(a.x + push.x, 0, a.z + push.z);
	CHECK_NEAR(Dist(moved, b), 1.0, 1e-4);
	CHECK(push.y == 0.0f);
	// 高さの違いは無視する（水平の距離だけで判定）
	const Vector3 high = SeparationPush(V3(0, 5, 0), V3(0.5f, -3, 0), 1.0f);
	CHECK(high.y == 0.0f);
	CHECK_NEAR(high.x, -0.5, 1e-5);
	// 中心が一致: 有限の向き（+X）
	const Vector3 same = SeparationPush(V3(3, 0, 3), V3(3, 0, 3), 1.0f);
	CHECK(NearlyEqual(same, V3(1, 0, 0)));
}

TEST_CASE(EnemyMath_SeparationPushIsSymmetric) {
	// 双方を半分ずつ動かせば、重なりが解ける（敵どうしの押し合い）
	const Vector3 a = V3(0, 0, 0), b = V3(0.5f, 0, 0.2f);
	const Vector3 pushA = SeparationPush(a, b, 1.0f);
	const Vector3 pushB = SeparationPush(b, a, 1.0f);
	CHECK(NearlyEqual(pushA, V3(-pushB.x, 0, -pushB.z), 1e-5f));
	const Vector3 a2 = V3(a.x + pushA.x * 0.5f, 0, a.z + pushA.z * 0.5f);
	const Vector3 b2 = V3(b.x + pushB.x * 0.5f, 0, b.z + pushB.z * 0.5f);
	CHECK_NEAR(Dist(a2, b2), 1.0, 1e-4);
}

TEST_CASE(EnemyMath_SpawnInScale) {
	CHECK_NEAR(SpawnInScale(0.0f, 0.4f), 0.0, 1e-6);
	CHECK_NEAR(SpawnInScale(0.4f, 0.4f), 1.0, 1e-6);
	CHECK_NEAR(SpawnInScale(0.2f, 0.4f), 0.5, 1e-6);   // 中間は半分
	CHECK_NEAR(SpawnInScale(5.0f, 0.4f), 1.0, 1e-6);   // 超えても1
	CHECK_NEAR(SpawnInScale(-1.0f, 0.4f), 0.0, 1e-6);  // 負でも0
	CHECK_NEAR(SpawnInScale(0.0f, 0.0f), 1.0, 1e-6);   // 時間0: すぐ1
	// 単調に増える
	float previous = -1.0f;
	bool monotonic = true;
	for (int i = 0; i <= 20; ++i) {
		const float s = SpawnInScale(i * 0.02f, 0.4f);
		monotonic = monotonic && s >= previous;
		previous = s;
	}
	CHECK(monotonic);
}

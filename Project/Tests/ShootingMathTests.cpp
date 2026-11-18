#include "TestFramework.h"
#include "Source/Application/Game/ShootingMath.h"

#include <cmath>

using namespace Engine;
using namespace Shooter;

namespace {

Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
AABB Box(float x0, float y0, float z0, float x1, float y1, float z1) { return AABB{ V3(x0, y0, z0), V3(x1, y1, z1) }; }

BulletState Bullet(const Vector3& position, const Vector3& velocity, float life) {
	BulletState b;
	b.position = position;
	b.velocity = velocity;
	b.lifeRemaining = life;
	return b;
}

// 一定のフレーム間隔で、押し続けたときの発射数
int CountShots(float seconds, float deltaTime, float interval, bool held = true) {
	float cooldown = 0.0f;
	int total = 0;
	const int frames = static_cast<int>(seconds / deltaTime + 0.5f);
	for (int i = 0; i < frames; ++i) {
		total += ConsumeShots(cooldown, deltaTime, held, interval);
	}
	return total;
}

} // namespace

// ---- 照準 ----

TEST_CASE(ShootingMath_AimDirectionFromPoint) {
	const Vector3 fallback = V3(0, 0, 1);
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(0, 0, 0), V3(5, 0, 0), fallback), V3(1, 0, 0)));
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(0, 0, 0), V3(0, 0, -3), fallback), V3(0, 0, -1)));
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(1, 0, 1), V3(4, 0, 5), fallback), V3(0.6f, 0, 0.8f))); // 3-4-5
	// 高さの違いは無視する（照準の点が、発射位置より高くても低くても、水平な向きになる）
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(0, 0, 0), V3(5, 9, 0), fallback), V3(1, 0, 0)));
	CHECK(AimDirectionFromPoint(V3(0, 1, 0), V3(5, -4, 0), fallback).y == 0.0f);
	// 近すぎて向きが不安定な場合は、fallback
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(0, 0, 0), V3(0.1f, 0, 0.1f), fallback), fallback));
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(2, 0, 2), V3(2, 7, 2), fallback), fallback)); // 真上
	CHECK(NearlyEqual(AimDirectionFromPoint(V3(0, 0, 0), V3(0.5f, 0, 0), fallback, 1.0f), fallback)); // しきい値を指定
}

TEST_CASE(ShootingMath_StickActive) {
	CHECK(!IsStickActive(Vector2{ 0.0f, 0.0f }, 0.3f));
	CHECK(!IsStickActive(Vector2{ 0.2f, 0.0f }, 0.3f));
	CHECK(IsStickActive(Vector2{ 0.0f, -0.5f }, 0.3f));
	CHECK(!IsStickActive(Vector2{ 0.3f, 0.0f }, 0.3f)); // ちょうどしきい値は、傾けていない扱い
	CHECK(IsStickActive(Vector2{ 0.25f, 0.25f }, 0.3f)); // 斜めは、長さで判定
}

TEST_CASE(ShootingMath_ChooseAimSource) {
	CHECK(ChooseAimSource(AimSource::kGamepad, true, false) == AimSource::kMouse);     // マウスが動いた
	CHECK(ChooseAimSource(AimSource::kMouse, false, true) == AimSource::kGamepad);     // スティックを傾けた
	CHECK(ChooseAimSource(AimSource::kMouse, false, false) == AimSource::kMouse);      // 何も操作なし: そのまま
	CHECK(ChooseAimSource(AimSource::kGamepad, false, false) == AimSource::kGamepad);
	CHECK(ChooseAimSource(AimSource::kMouse, true, true) == AimSource::kMouse);        // 同時: そのまま
	CHECK(ChooseAimSource(AimSource::kGamepad, true, true) == AimSource::kGamepad);
}

// ---- 連射 ----

TEST_CASE(ShootingMath_FirstShotIsImmediate) {
	float cooldown = 0.0f;
	CHECK(ConsumeShots(cooldown, 1.0f / 60.0f, true, 0.1f) == 1); // 押した最初のフレームで、すぐ撃てる
	CHECK(ConsumeShots(cooldown, 1.0f / 60.0f, true, 0.1f) == 0); // 次のフレームは、まだ
}

TEST_CASE(ShootingMath_NotHeldDoesNotFire) {
	float cooldown = 0.0f;
	CHECK(ConsumeShots(cooldown, 0.016f, false, 0.1f) == 0);
	CHECK(cooldown == 0.0f);
	CHECK(CountShots(5.0f, 1.0f / 60.0f, 0.1f, false) == 0);
}

TEST_CASE(ShootingMath_FireRateIsFrameRateIndependent) {
	// 10秒間・間隔0.1秒 → 約100発。フレームレートが違っても、ほぼ同じ（最初の1発ぶんの誤差のみ）
	for (float dt : { 1.0f / 144.0f, 1.0f / 60.0f, 1.0f / 30.0f, 1.0f / 15.0f }) {
		int shots = CountShots(10.0f, dt, 0.1f);
		CHECK(shots >= 99 && shots <= 101);
	}
	// 間隔がフレームより短い（1フレームに複数発）場合も、平均は保たれる
	int fast = CountShots(10.0f, 1.0f / 15.0f, 0.03f);
	CHECK(fast >= 330 && fast <= 335);
}

TEST_CASE(ShootingMath_BurstIsCapped) {
	float cooldown = 0.0f;
	CHECK(ConsumeShots(cooldown, 5.0f, true, 0.1f) == 4);       // 長い処理落ちのあとも、一度に大量には出ない
	CHECK(ConsumeShots(cooldown, 0.016f, true, 0.1f) <= 1);     // 持ち越しもしない
	float cooldown2 = 0.0f;
	CHECK(ConsumeShots(cooldown2, 5.0f, true, 0.1f, 2) == 2);   // 上限は指定できる
}

TEST_CASE(ShootingMath_ReleaseAndPressAgain) {
	float cooldown = 0.0f;
	CHECK(ConsumeShots(cooldown, 0.016f, true, 0.5f) == 1);
	CHECK(ConsumeShots(cooldown, 0.016f, false, 0.5f) == 0);   // 離す: 待ち時間は、時間とともに減る
	for (int i = 0; i < 40; ++i) (void)ConsumeShots(cooldown, 0.016f, false, 0.5f); // しばらく離しておく
	CHECK(cooldown == 0.0f);
	CHECK(ConsumeShots(cooldown, 0.016f, true, 0.5f) == 1);    // 押した瞬間に、すぐ出る
	// 連打（すぐ離してすぐ押す）で、間隔を無視して連射できてしまわない
	float tapping = 0.0f;
	int shots = 0;
	for (int i = 0; i < 20; ++i) {
		shots += ConsumeShots(tapping, 0.016f, (i % 2) == 0, 0.5f); // 1フレームおきに押す・離す（約0.3秒）
	}
	CHECK(shots == 1);
}

// ---- 弾 ----

TEST_CASE(ShootingMath_BulletFliesStraight) {
	BulletState bullet = Bullet(V3(0, 1, 0), V3(10, 0, 0), 2.0f);
	BulletStepResult r = AdvanceBullet(bullet, 0.5f, nullptr, 0, 0.15f);
	CHECK(r.alive);
	CHECK(!r.hitWall);
	CHECK(NearlyEqual(bullet.position, V3(5, 1, 0)));
	CHECK_NEAR(bullet.lifeRemaining, 1.5, 1e-6);
}

TEST_CASE(ShootingMath_BulletExpires) {
	BulletState bullet = Bullet(V3(0, 1, 0), V3(1, 0, 0), 0.3f);
	CHECK(AdvanceBullet(bullet, 0.2f, nullptr, 0, 0.15f).alive);
	CHECK(!AdvanceBullet(bullet, 0.2f, nullptr, 0, 0.15f).alive); // 寿命が尽きた
}

TEST_CASE(ShootingMath_BulletStopsAtWallFace) {
	const AABB wall = Box(10, 0, -5, 12, 3, 5);
	BulletState bullet = Bullet(V3(0, 1, 0), V3(20, 0, 0), 5.0f);
	// 1フレームで 1m ずつ進む弾が、壁の手前（弾の半径ぶん）で止まる
	BulletStepResult r;
	for (int i = 0; i < 100 && bullet.position.x < 9.0f; ++i) {
		r = AdvanceBullet(bullet, 0.05f, &wall, 1, 0.15f);
	}
	r = AdvanceBullet(bullet, 0.05f, &wall, 1, 0.15f);
	CHECK(r.hitWall);
	CHECK(!r.alive);
	CHECK_NEAR(r.hitPoint.x, 9.85, 1e-4);          // 壁の面(10) - 半径(0.15)
	CHECK(NearlyEqual(r.hitNormal, V3(-1, 0, 0))); // 手前の面
	CHECK_NEAR(bullet.position.x, 9.85, 1e-4);
}

TEST_CASE(ShootingMath_FastBulletDoesNotTunnelThroughWall) {
	// 1フレームで、薄い壁（厚さ0.2）を飛び越える速さでも、当たる
	const AABB thinWall = Box(5.0f, 0, -5, 5.2f, 3, 5);
	BulletState bullet = Bullet(V3(0, 1, 0), V3(600, 0, 0), 5.0f);
	BulletStepResult r = AdvanceBullet(bullet, 0.1f, &thinWall, 1, 0.1f); // 1フレームで 60m 進む
	CHECK(r.hitWall);
	CHECK_NEAR(r.hitPoint.x, 4.9, 1e-4);
	// 斜めに飛ぶ弾も、同じ
	BulletState diagonal = Bullet(V3(0, 1, -2), V3(300, 0, 100), 5.0f);
	CHECK(AdvanceBullet(diagonal, 0.1f, &thinWall, 1, 0.1f).hitWall);
}

TEST_CASE(ShootingMath_BulletPicksNearestWall) {
	const AABB walls[2] = { Box(20, 0, -5, 22, 3, 5), Box(10, 0, -5, 12, 3, 5) }; // 遠い壁が、先に並んでいる
	BulletState bullet = Bullet(V3(0, 1, 0), V3(500, 0, 0), 5.0f);
	BulletStepResult r = AdvanceBullet(bullet, 0.1f, walls, 2, 0.15f);
	CHECK(r.hitWall);
	CHECK_NEAR(r.hitPoint.x, 9.85, 1e-4);
}

TEST_CASE(ShootingMath_BulletMissesWall) {
	const AABB wall = Box(10, 0, -5, 12, 3, 5);
	// 壁の脇を通る
	BulletState beside = Bullet(V3(0, 1, 8), V3(100, 0, 0), 5.0f);
	CHECK(!AdvanceBullet(beside, 0.5f, &wall, 1, 0.15f).hitWall);
	// 壁から遠ざかる
	BulletState away = Bullet(V3(5, 1, 0), V3(-100, 0, 0), 5.0f);
	CHECK(!AdvanceBullet(away, 0.5f, &wall, 1, 0.15f).hitWall);
	// 壁の上を越える（壁の高さより上）
	BulletState over = Bullet(V3(0, 4.0f, 0), V3(100, 0, 0), 5.0f);
	CHECK(!AdvanceBullet(over, 0.5f, &wall, 1, 0.15f).hitWall);
	// 弾の半径ぶん、壁の脇をかすめると当たる（中心は壁の外側 0.1）
	BulletState grazing = Bullet(V3(0, 1, 5.1f), V3(100, 0, 0), 5.0f);
	CHECK(AdvanceBullet(grazing, 0.5f, &wall, 1, 0.15f).hitWall);
}

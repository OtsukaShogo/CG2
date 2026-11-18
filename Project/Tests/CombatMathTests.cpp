#include "TestFramework.h"
#include "Source/Application/Game/CombatMath.h"

#include <cmath>

using namespace Engine;
using namespace Shooter;

namespace {

Vector3 V3(float x, float y, float z) { return Vector3{ x, y, z }; }
Segment3D Seg(float x0, float y0, float z0, float x1, float y1, float z1) { return Segment3D{ V3(x0, y0, z0), V3(x1, y1, z1) }; }

// 敵と同じ形（半径0.45・高さ1.5）のカプセル
Capsule EnemyAt(float x, float z) { return MakeUprightCapsule(V3(x, 0, z), 1.5f, 0.45f); }

} // namespace

TEST_CASE(CombatMath_HealthBasics) {
	Health h = MakeHealth(5);
	CHECK(h.hp == 5);
	CHECK(h.maxHp == 5);
	CHECK(!IsDead(h));
	CHECK_NEAR(HealthRatio(h), 1.0, 1e-6);
	CHECK(TakeDamage(h, 2));
	CHECK(h.hp == 3);
	CHECK_NEAR(HealthRatio(h), 0.6, 1e-6);
}

TEST_CASE(CombatMath_DamageClampsAtZeroAndKills) {
	Health h = MakeHealth(3);
	CHECK(TakeDamage(h, 10));  // 過剰なダメージ: 体力は負にならない
	CHECK(h.hp == 0);
	CHECK(IsDead(h));
	CHECK_NEAR(HealthRatio(h), 0.0, 1e-6);
	CHECK(!TakeDamage(h, 1));  // 死んでいる: もう受けない
	CHECK(h.hp == 0);
}

TEST_CASE(CombatMath_InvalidDamageIsIgnored) {
	Health h = MakeHealth(3);
	CHECK(!TakeDamage(h, 0));
	CHECK(!TakeDamage(h, -5));  // 負のダメージで、回復はしない
	CHECK(h.hp == 3);
	CHECK(h.invulnerableTime == 0.0f);
}

TEST_CASE(CombatMath_Invulnerability) {
	Health h = MakeHealth(5);
	CHECK(TakeDamage(h, 1, 1.0f));               // 1回目: 受ける。1秒の無敵
	CHECK(h.hp == 4);
	CHECK(!TakeDamage(h, 1, 1.0f));              // 無敵中: 受けない
	CHECK(h.hp == 4);
	UpdateHealth(h, 0.6f);
	CHECK(!TakeDamage(h, 1, 1.0f));              // まだ無敵（残り0.4秒）
	UpdateHealth(h, 0.5f);
	CHECK(h.invulnerableTime == 0.0f);           // 0未満にならない
	CHECK(TakeDamage(h, 1, 1.0f));               // 無敵が切れたので、受ける
	CHECK(h.hp == 3);
}

TEST_CASE(CombatMath_TouchingForeverDoesNotKillInstantly) {
	// 触れ続けても、無敵時間の間隔でしかダメージを受けない（10秒触れて、10回）
	Health h = MakeHealth(100);
	int hits = 0;
	for (int i = 0; i < 600; ++i) { // 60fps で 10秒
		UpdateHealth(h, 1.0f / 60.0f);
		hits += TakeDamage(h, 1, 1.0f) ? 1 : 0;
	}
	CHECK(hits >= 10 && hits <= 11);
}

TEST_CASE(CombatMath_NoInvulnerabilityMeansEveryHit) {
	// 敵の体力のように、無敵時間なしなら、当たるたびにダメージ
	Health h = MakeHealth(3);
	CHECK(TakeDamage(h, 1, 0.0f));
	CHECK(TakeDamage(h, 1, 0.0f));
	CHECK(TakeDamage(h, 1, 0.0f));
	CHECK(IsDead(h));
}

TEST_CASE(CombatMath_HealthRatioEdgeCases) {
	Health none;
	CHECK_NEAR(HealthRatio(none), 0.0, 1e-9); // 最大体力0
	Health over = MakeHealth(4);
	over.hp = 9;                                 // 最大を超えていても、1まで
	CHECK_NEAR(HealthRatio(over), 1.0, 1e-9);
}

TEST_CASE(CombatMath_BlinkPattern) {
	CHECK(IsBlinkVisible(0.0f, 0.1f));    // 無敵でなければ、常に表示
	CHECK(IsBlinkVisible(0.05f, 0.1f));   // 終わる直前の区間は、表示
	CHECK(!IsBlinkVisible(0.15f, 0.1f));  // 次の区間は、非表示
	CHECK(IsBlinkVisible(0.25f, 0.1f));
	CHECK(!IsBlinkVisible(0.35f, 0.1f));
	CHECK(IsBlinkVisible(0.5f, 0.0f));    // 周期0は、点滅しない
	// 1秒間の無敵の間に、表示と非表示が、同じくらいの割合で切り替わる
	int visible = 0, hidden = 0;
	for (int i = 0; i < 1000; ++i) {
		(IsBlinkVisible(1.0f - i / 1000.0f, 0.08f) ? visible : hidden)++;
	}
	CHECK(visible > 400 && hidden > 400);
}

TEST_CASE(CombatMath_Contact) {
	CHECK(IsInContact(V3(0, 0, 0), V3(0.9f, 0, 0), 0.95f, 0.0f));     // 半径の和以内
	CHECK(!IsInContact(V3(0, 0, 0), V3(1.0f, 0, 0), 0.95f, 0.0f));
	CHECK(IsInContact(V3(0, 0, 0), V3(1.0f, 0, 0), 0.95f, 0.1f));     // 余裕を持たせれば、接触
	CHECK(IsInContact(V3(0, 5, 0), V3(0.5f, -3, 0), 0.95f, 0.0f));    // 高さは無視
	CHECK(!IsInContact(V3(0, 0, 0), V3(3, 0, 4), 0.95f, 0.1f));       // 距離5
	// ちょうど接している（押し合いの結果）状態が、誤差で途切れない
	const float radiusSum = 0.45f + 0.5f;
	CHECK(IsInContact(V3(0, 0, 0), V3(radiusSum + 1e-4f, 0, 0), radiusSum, 0.1f));
}

TEST_CASE(CombatMath_BulletHitsCapsule) {
	const Capsule enemy = EnemyAt(10, 0); // 半径0.45・高さ1.5（弾の高さ1.0は、胴体の範囲内）
	float t = -1.0f;
	// 正面から
	CHECK(BulletHitsCapsule(Seg(0, 1, 0, 20, 1, 0), 0.15f, enemy, &t));
	CHECK_NEAR(t, (10.0 - 0.45 - 0.15) / 20.0, 1e-4); // 手前の面（半径 + 弾の半径）で当たる
	// 外れる / 届かない / 逆向き
	CHECK(!BulletHitsCapsule(Seg(0, 1, 3, 20, 1, 3), 0.15f, enemy, &t));
	CHECK(!BulletHitsCapsule(Seg(0, 1, 0, 5, 1, 0), 0.15f, enemy, &t));
	CHECK(!BulletHitsCapsule(Seg(20, 1, 0, 30, 1, 0), 0.15f, enemy, &t));
}

TEST_CASE(CombatMath_BulletGrazingUsesBulletRadius) {
	const Capsule enemy = EnemyAt(10, 0);
	// 中心が、敵の半径(0.45)の外側 0.1 をかすめる: 弾の半径(0.15)ぶん太らせると、当たる
	CHECK(BulletHitsCapsule(Seg(0, 1, 0.55f, 20, 1, 0.55f), 0.15f, enemy, nullptr));
	// 0.7 離れていれば、外れる（0.45 + 0.15 = 0.6 < 0.7）
	CHECK(!BulletHitsCapsule(Seg(0, 1, 0.7f, 20, 1, 0.7f), 0.15f, enemy, nullptr));
}

TEST_CASE(CombatMath_FastBulletDoesNotTunnelThroughEnemy) {
	const Capsule enemy = EnemyAt(10, 0);
	// 1フレームで、敵の反対側まで抜ける線分でも、当たる
	CHECK(BulletHitsCapsule(Seg(5, 1, 0, 400, 1, 0), 0.15f, enemy, nullptr));
	// 斜めに通り抜ける
	CHECK(BulletHitsCapsule(Seg(0, 1, -5, 20, 1, 5), 0.15f, enemy, nullptr));
}

TEST_CASE(CombatMath_BulletOverOrUnderEnemyMisses) {
	const Capsule enemy = EnemyAt(10, 0); // 高さ0〜1.5
	CHECK(!BulletHitsCapsule(Seg(0, 2.5f, 0, 20, 2.5f, 0), 0.15f, enemy, nullptr));  // 頭の上を通る
	CHECK(!BulletHitsCapsule(Seg(0, -1.0f, 0, 20, -1.0f, 0), 0.15f, enemy, nullptr)); // 地面の下
	CHECK(BulletHitsCapsule(Seg(0, 1.55f, 0, 20, 1.55f, 0), 0.15f, enemy, nullptr));  // 頭の上すれすれ（弾の半径ぶん）
}

TEST_CASE(CombatMath_BulletStartingInsideEnemyHits) {
	const Capsule enemy = EnemyAt(10, 0);
	float t = -1.0f;
	CHECK(BulletHitsCapsule(Seg(10, 1, 0, 20, 1, 0), 0.15f, enemy, &t)); // 銃口が、敵にめり込んでいる（至近距離）
	CHECK(t == 0.0f);
}

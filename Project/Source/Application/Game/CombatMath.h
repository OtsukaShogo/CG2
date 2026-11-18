#pragma once
#include "../../Engine/Collision/Collision3D.h"
#include "../../Engine/Math/Vector3.h"

// 戦闘まわりの計算（D3D12に依存しない、単体テストできる部分）。
namespace Shooter {

/// <summary>
/// 体力。ダメージを受けたあとの無敵時間も持つ（自機が、触れている間じゅう、毎フレームダメージを受けないように）
/// </summary>
struct Health {
	int hp = 0;
	int maxHp = 0;
	float invulnerableTime = 0.0f; //!< 残りの無敵時間（秒）。0より大きい間は、ダメージを受けない
};

/// <summary>
/// 最大体力を指定して、満タンの体力を作る
/// </summary>
[[nodiscard]] Health MakeHealth(int maxHp);

/// <summary>
/// 体力が尽きているか
/// </summary>
[[nodiscard]] bool IsDead(const Health& health);

/// <summary>
/// ダメージを与える。すでに死んでいる・無敵中・ダメージが0以下の場合は、何もしない。
/// 体力は、0未満にならない。与えられたら、invulnerableDuration 秒の無敵になる
/// </summary>
/// <returns>ダメージを与えられたら true</returns>
bool TakeDamage(Health& health, int amount, float invulnerableDuration = 0.0f);

/// <summary>
/// 無敵時間を、経過時間ぶん減らす（0未満にならない）
/// </summary>
void UpdateHealth(Health& health, float deltaTime);

/// <summary>
/// 体力の割合（0〜1）。最大体力が0以下なら 0
/// </summary>
[[nodiscard]] float HealthRatio(const Health& health);

/// <summary>
/// 無敵の間の点滅で、いま表示するべきか。無敵でなければ、常に表示する。
/// 無敵の間は、残り時間を period 秒ごとに区切り、終わりに近い区間から数えて、偶数番目は表示、奇数番目は非表示にする
/// （交互に点滅する。無敵が終わる直前の区間は、必ず表示なので、終わったときに、見た目が途切れない）
/// </summary>
/// <param name="invulnerableTime">残りの無敵時間</param>
/// <param name="period">点滅の周期（表示、または非表示の、1回ぶんの長さ）</param>
[[nodiscard]] bool IsBlinkVisible(float invulnerableTime, float period);

/// <summary>
/// 水平な円どうし（中心 a・b）が、接触しているか。半径の和に、margin を足した距離以内なら true。
/// 押し合いで「ちょうど接した」状態になっていても、誤差で接触が途切れないよう、少し余裕を持たせる
/// </summary>
[[nodiscard]] bool IsInContact(const Engine::Vector3& a, const Engine::Vector3& b, float radiusSum, float margin);

/// <summary>
/// 弾が1フレームで進んだ道のり（線分）が、標的のカプセルに当たるか。
/// 弾の大きさぶん標的を太らせて調べるので、細い弾でも、速くても、すり抜けない。
/// 道のりの始点がすでに標的の中にあれば、t=0 で当たる
/// </summary>
/// <param name="outT">当たった位置（0=始点、1=終点）</param>
[[nodiscard]] bool BulletHitsCapsule(const Engine::Segment3D& path, float bulletRadius, const Engine::Capsule& target, float* outT);

} // namespace Shooter

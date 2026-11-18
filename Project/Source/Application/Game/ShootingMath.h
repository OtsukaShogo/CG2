#pragma once
#include <cstddef>
#include "../../Engine/Collision/Collision3D.h"
#include "../../Engine/Math/Vector2.h"
#include "../../Engine/Math/Vector3.h"

// 射撃まわりの計算（D3D12に依存しない、単体テストできる部分）。
namespace Shooter {

/// <summary>
/// 照準を決めている入力装置
/// </summary>
enum class AimSource {
	kMouse,
	kGamepad,
};

/// <summary>
/// 発射位置から照準の点へ向かう、水平な向き（XZ平面、長さ1、yは0）を求める。
/// 照準の点が発射位置に近すぎて向きが不安定な場合（カーソルが自機の真上など）は、fallback を返す
/// </summary>
/// <param name="minDistance">この距離（水平方向）より近い場合は、fallback を返す</param>
[[nodiscard]] Engine::Vector3 AimDirectionFromPoint(const Engine::Vector3& from, const Engine::Vector3& aimPoint, const Engine::Vector3& fallback, float minDistance = 0.3f);

/// <summary>
/// スティックが、狙う入力として傾けられているか（長さが threshold を超えているか）
/// </summary>
[[nodiscard]] bool IsStickActive(const Engine::Vector2& stick, float threshold);

/// <summary>
/// 照準の入力装置を選ぶ。直近で操作された装置に切り替える。
/// マウスとスティックのどちらも操作されていない場合、または、同時に操作された場合は、今のままにする
/// </summary>
[[nodiscard]] AimSource ChooseAimSource(AimSource current, bool mouseActive, bool stickActive);

/// <summary>
/// 連射の間隔を管理し、このフレームで何発撃つかを返す。
/// 間隔が短いときや、フレームが遅れたときも、平均の連射速度が保たれる（フレームをまたいで、余りを持ち越す）。
/// 押していない間は、待ち時間が0に戻るので、押した瞬間に、すぐ1発目が出る
/// </summary>
/// <param name="cooldown">次に撃てるまでの残り時間（呼び出しをまたいで保持する。最初は0）</param>
/// <param name="wantsToFire">発射ボタンが押されているか</param>
/// <param name="interval">連射の間隔（秒）</param>
/// <param name="maxShots">1フレームに撃てる最大数（処理落ちのあとに、弾が一度に大量に出るのを防ぐ）</param>
[[nodiscard]] int ConsumeShots(float& cooldown, float deltaTime, bool wantsToFire, float interval, int maxShots = 4);

/// <summary>
/// 弾の状態（位置・速度・残りの寿命）
/// </summary>
struct BulletState {
	Engine::Vector3 position = { 0.0f, 0.0f, 0.0f };
	Engine::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	float lifeRemaining = 0.0f;
};

/// <summary>
/// 弾を1フレーム進めた結果
/// </summary>
struct BulletStepResult {
	bool alive = true;      //!< まだ飛び続けるか（寿命切れ・壁に当たった場合は false）
	bool hitWall = false;
	Engine::Vector3 hitPoint = { 0.0f, 0.0f, 0.0f };  //!< 壁に当たった位置
	Engine::Vector3 hitNormal = { 0.0f, 0.0f, 0.0f }; //!< 当たった壁の面の向き
};

/// <summary>
/// 弾を1フレーム進める。前の位置から次の位置までを線分として、壁（半径ぶん太らせた箱）との交差を調べるので、
/// 速い弾が、1フレームで壁をすり抜けない。壁に当たったら、当たった位置で止まる
/// </summary>
/// <param name="walls">壁の判定</param>
/// <param name="radius">弾の半径</param>
BulletStepResult AdvanceBullet(BulletState& bullet, float deltaTime, const Engine::AABB* walls, size_t wallCount, float radius);

} // namespace Shooter

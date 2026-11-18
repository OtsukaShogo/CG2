#pragma once
#include <random>
#include "../../Engine/Math/Vector3.h"

// 敵まわりの計算（D3D12に依存しない、単体テストできる部分）。水平面は XZ 平面。
namespace Shooter {

/// <summary>
/// from から to へ向かう、水平な向き（XZ平面、長さ1、yは0）を求める。水平方向に距離がなければ {0,0,0}
/// </summary>
[[nodiscard]] Engine::Vector3 DirectionToTarget(const Engine::Vector3& from, const Engine::Vector3& to);

/// <summary>
/// 敵の出現のタイミングを管理し、このフレームで何体出すかを返す。
/// 生きている敵が上限に達している間は出さない（待ち時間は進めるので、上限を下回った瞬間に出る）。
/// </summary>
/// <param name="timer">次に出すまでの残り時間（呼び出しをまたいで保持する。最初は0 = すぐ1体目が出る）</param>
/// <param name="interval">出現の間隔（秒）</param>
/// <param name="aliveCount">いま生きている敵の数</param>
/// <param name="maxAlive">同時に生きていられる上限</param>
/// <param name="maxPerFrame">1フレームに出す最大数（処理落ちのあとに、一度に大量に出るのを防ぐ）</param>
[[nodiscard]] int ConsumeSpawns(float& timer, float deltaTime, float interval, int aliveCount, int maxAlive, int maxPerFrame = 2);

/// <summary>
/// アリーナの外周（壁から margin だけ内側の、長方形の辺の上）から、敵の出現位置を選ぶ。
/// 自機から minDistance 以上離れた位置を、一様に近い確率で選ぶ（外周上の位置を、何度か引き直して探す）。
/// どうしても見つからない場合（自機の周りが、すべて近い）は、自機から最も遠い角を返す。必ず、外周上の位置を返す
/// </summary>
/// <param name="halfWidth">アリーナの、中心から壁の内側までの長さ（X）</param>
/// <param name="halfDepth">同（Z）</param>
/// <param name="margin">壁からの内側への余裕（敵の体が、壁にめり込まないように）</param>
/// <param name="playerPosition">自機の位置</param>
/// <param name="minDistance">自機から、これ以上離れた位置を選ぶ</param>
[[nodiscard]] Engine::Vector3 ChooseSpawnPoint(std::mt19937& random, float halfWidth, float halfDepth, float margin,
	const Engine::Vector3& playerPosition, float minDistance);

/// <summary>
/// 外周の長方形の上の、位置を求める。t は 0〜1（外周を1周する割合）。
/// 内側の長方形（半分の大きさ halfWidth - margin, halfDepth - margin）の辺を、左手前の角から時計回りに進む
/// </summary>
[[nodiscard]] Engine::Vector3 PerimeterPoint(float t, float halfWidth, float halfDepth, float margin);

/// <summary>
/// 水平な円どうし（中心 a・b、半径の和 radiusSum）が重なっているとき、a を動かして重なりを解く量（水平、y は 0）を返す。
/// 重なっていなければ {0,0,0}。中心が一致していて向きが決まらない場合は、+X へ動かす
/// </summary>
[[nodiscard]] Engine::Vector3 SeparationPush(const Engine::Vector3& a, const Engine::Vector3& b, float radiusSum);

/// <summary>
/// 出現してからの時間 age から、出現演出の大きさの倍率（0〜1）を求める。0から始まり、滑らかに 1 になる
/// </summary>
/// <param name="duration">出現演出にかかる時間（0以下なら、すぐ1）</param>
[[nodiscard]] float SpawnInScale(float age, float duration);

} // namespace Shooter

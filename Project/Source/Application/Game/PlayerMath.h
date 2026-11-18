#pragma once
#include "../../Engine/Math/Vector2.h"
#include "../../Engine/Math/Vector3.h"

// 自機の移動まわりの計算（D3D12に依存しない、単体テストできる部分）。
// 水平面は XZ 平面。Vector2 の x がワールドの X、y がワールドの Z に対応する。
namespace Shooter {

/// <summary>
/// 移動に関する入力（キーボードとゲームパッドを、まとめて表す）
/// </summary>
struct MoveInput {
	bool left = false;
	bool right = false;
	bool up = false;    //!< 画面の奥へ
	bool down = false;
	Engine::Vector2 stick = { 0.0f, 0.0f }; //!< 左スティック（x:右が正、y:上が正。長さ0〜1）
};

/// <summary>
/// キーボードとスティックの入力を1つにまとめる。
/// - キーボードは、斜め押しでも長さ1になる（斜めが速くならない）。
/// - スティックは、傾けた大きさがそのまま速さになる。
/// - 両方が入力されている場合は、足し合わせて、長さ1までに収める。
/// 戻り値は、x:右が正、y:上（画面の奥）が正で、長さ0〜1
/// </summary>
[[nodiscard]] Engine::Vector2 CombineMoveInput(const MoveInput& input);

/// <summary>
/// 3Dのベクトルを水平面（XZ）に投影して、長さ1にする。真上・真下を向いていて水平成分がなければ {0,0}
/// </summary>
[[nodiscard]] Engine::Vector2 ToGroundDirection(const Engine::Vector3& v);

/// <summary>
/// 画面基準の入力（右・奥）を、カメラの向きに合わせた、ワールドの移動方向（XZ平面、yは0）にする。
/// 入力の長さ（0〜1）は保たれる。カメラを回転させても、「画面の奥」へ進むようになる
/// </summary>
/// <param name="input">CombineMoveInput の結果</param>
/// <param name="screenRight">画面の右方向が、地面上で向いている向き（長さ1）</param>
/// <param name="screenUp">画面の上（奥）方向が、地面上で向いている向き（長さ1）</param>
[[nodiscard]] Engine::Vector3 MoveInputToWorld(const Engine::Vector2& input, const Engine::Vector2& screenRight, const Engine::Vector2& screenUp);

/// <summary>
/// 現在の速度を、目標の速度へ近づける（1フレームで、最大 maxDelta だけ変化する）。加減速の滑らかさになる
/// </summary>
[[nodiscard]] Engine::Vector3 ApproachVelocity(const Engine::Vector3& current, const Engine::Vector3& target, float maxDelta);

/// <summary>
/// 水平な向きベクトルから、Y軸まわりの角度（ラジアン）を求める。+Z が 0、+X が π/2。
/// 水平成分がなければ fallback を返す
/// </summary>
[[nodiscard]] float YawFromDirection(const Engine::Vector3& direction, float fallback);

/// <summary>
/// 角度 current を、target へ向けて、最大 maxStep だけ回す（最短の向きで回る）
/// </summary>
[[nodiscard]] float RotateTowardsAngle(float current, float target, float maxStep);

} // namespace Shooter

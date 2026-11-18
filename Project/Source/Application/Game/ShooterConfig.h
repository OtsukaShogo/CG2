#pragma once

// ゲームの調整値をまとめた場所。遊びながら調整する数値は、ここに集める。
// 単位は、長さが「ワールドの1単位」、時間が「秒」。
namespace Shooter::Config {

// ---- アリーナ（壁で囲まれた、固定の戦場） ----
inline constexpr float kArenaHalfWidth = 20.0f; //!< X方向の、中心から壁の内側までの長さ
inline constexpr float kArenaHalfDepth = 14.0f; //!< Z方向の、中心から壁の内側までの長さ
inline constexpr float kWallThickness = 2.0f;
inline constexpr float kWallHeight = 3.0f;

// ---- 自機 ----
inline constexpr float kPlayerRadius = 0.5f;       //!< カプセルの半径
inline constexpr float kPlayerHeight = 1.8f;       //!< カプセルの全体の高さ
inline constexpr float kPlayerSpeed = 7.0f;        //!< 最高速度
inline constexpr float kPlayerAcceleration = 60.0f; //!< 加速（1秒あたりの速度の変化）
inline constexpr float kPlayerDeceleration = 80.0f; //!< 減速（入力をやめたとき。加速より強く、止まりやすくする）
inline constexpr float kPlayerTurnSpeed = 14.0f;   //!< 向きを変える速さ（ラジアン/秒）

// ---- カメラ（斜めから、自機を追従する平行投影） ----
inline constexpr float kCameraPitch = 0.95f;       //!< 見下ろす角度（ラジアン。π/2 で真上から）
inline constexpr float kCameraDistance = 30.0f;    //!< 注視点からの距離（平行投影なので、見た目の大きさには影響しない）
inline constexpr float kCameraOrthoHeight = 16.0f; //!< 画面の縦に映す範囲（ワールドの長さ）
inline constexpr float kCameraFollowSpeed = 8.0f;  //!< 追従の速さ

} // namespace Shooter::Config

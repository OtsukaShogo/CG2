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

// ---- 射撃 ----
inline constexpr float kBulletSpeed = 26.0f;        //!< 弾の速さ
inline constexpr float kBulletLifetime = 1.5f;      //!< 弾が飛び続ける時間（秒）
inline constexpr float kBulletRadius = 0.15f;       //!< 弾の半径（壁との判定にも使う）
inline constexpr float kBulletHeight = 1.0f;        //!< 弾を撃つ高さ。マウスの照準も、この高さの水平面で判定する
inline constexpr float kFireInterval = 0.12f;       //!< 連射の間隔（秒）
inline constexpr int kBulletPoolCapacity = 256;     //!< 同時に飛べる弾の最大数
inline constexpr float kMuzzleDistance = kPlayerRadius + 0.3f; //!< 自機の中心から、弾が出る位置までの距離
inline constexpr float kAimStickThreshold = 0.25f;  //!< 右スティックを、狙う操作とみなす傾き
inline constexpr float kPadReticleDistance = 6.0f;  //!< ゲームパッドで狙うとき、照準を表示する自機からの距離
inline constexpr float kReticleRadius = 0.5f;       //!< 照準の表示の大きさ

} // namespace Shooter::Config

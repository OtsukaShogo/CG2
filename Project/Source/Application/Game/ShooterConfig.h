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

// ---- 敵 ----
inline constexpr float kEnemyRadius = 0.45f;          //!< 敵の体の半径（敵どうし・自機・壁との押し合いに使う）
inline constexpr float kEnemyHeight = 1.5f;           //!< 敵の体の高さ
inline constexpr float kEnemySpeed = 3.5f;            //!< 追いかける速さ（自機の最高速度 7 より遅く、逃げられる）
inline constexpr float kEnemyAcceleration = 25.0f;    //!< 加速（向きを変えるときも、急には曲がらない）
inline constexpr float kEnemyTurnSpeed = 10.0f;       //!< 向きを変える速さ（ラジアン/秒）
inline constexpr int kEnemyPoolCapacity = 128;        //!< 同時に存在できる敵の最大数（プールの容量）
inline constexpr int kEnemyMaxAlive = 40;             //!< 出現で増やす、生きている敵の上限（プールの容量より少なくする）
inline constexpr float kEnemySpawnInterval = 0.8f;    //!< 出現の間隔（秒）
inline constexpr float kEnemySpawnMargin = 1.5f;      //!< 出現位置の、壁からの内側への余裕
inline constexpr float kEnemySpawnMinDistance = 14.0f; //!< 出現位置を、自機からこれ以上離す
inline constexpr float kEnemySpawnInTime = 0.4f;      //!< 出現演出の時間（この間は、動かず、押し合いもしない）

// ---- 戦闘 ----
inline constexpr int kEnemyHp = 3;                    //!< 敵の体力（弾のダメージ 1 で、3発で倒れる）
inline constexpr int kBulletDamage = 1;               //!< 弾1発のダメージ
inline constexpr float kEnemyHitFlashTime = 0.09f;    //!< 敵が弾を受けたとき、白く光る時間（秒）
inline constexpr int kPlayerMaxHp = 5;                //!< 自機の体力
inline constexpr int kContactDamage = 1;              //!< 敵に触れたときのダメージ
inline constexpr float kPlayerInvulnerableTime = 1.0f; //!< 被弾したあとの無敵時間（秒）。触れ続けても、この間隔でしかダメージを受けない
inline constexpr float kContactMargin = 0.12f;        //!< 接触とみなす距離の余裕（押し合いで、ちょうど接した状態でも、途切れないように）
inline constexpr float kBlinkPeriod = 0.08f;          //!< 無敵中の点滅の周期（秒）

// ---- HUD（画面の左上に出す、体力のバー。ピクセル単位） ----
inline constexpr float kHudMargin = 24.0f;
inline constexpr float kHpBarWidth = 280.0f;
inline constexpr float kHpBarHeight = 24.0f;
inline constexpr float kHpBarBorder = 3.0f;

// ---- ゲームの流れ（シーンの遷移） ----
inline constexpr float kFadeInTime = 0.4f;          //!< シーンの開始時の、明るくなる時間（秒）
inline constexpr float kFadeOutTime = 0.35f;        //!< シーンを出るときの、暗くなる時間（秒）
inline constexpr float kSceneInputLockTime = 0.45f; //!< シーンの開始直後に、操作を受け付けない時間（前のシーンのクリックなどが、誤って効かないように）
inline constexpr float kDeathToResultDelay = 1.6f;  //!< 倒れてから、ゲームオーバー画面へ進むまでの時間（秒）
inline constexpr float kDeathSkipLockTime = 0.5f;   //!< 倒れた直後に、決定ボタンで先へ進めない時間（連射のクリックで、飛ばしてしまわないように）

// ---- 演出・音 ----
inline constexpr float kBulletTrailInterval = 0.03f; //!< 弾が、軌跡のパーティクルを残す間隔（秒）
inline constexpr float kPlayerChestHeight = 1.0f;    //!< 自機の被弾の演出を出す高さ（胸の高さ）

} // namespace Shooter::Config

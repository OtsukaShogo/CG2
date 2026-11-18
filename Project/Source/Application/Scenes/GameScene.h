#pragma once
#include <memory>
#include "../../Engine/Scene/Scene.h"
#include "../../Engine/3D/Camera.h"
#include "../../Engine/Graphics/Light.h"
#include "../Game/Arena.h"
#include "../Game/BulletManager.h"
#include "../Game/EnemyManager.h"
#include "../Game/Player.h"
#include "../Game/ShootingMath.h"

/// <summary>
/// ゲーム本体のシーン。壁で囲まれたアリーナで、自機を動かし、狙って弾を撃つ
/// </summary>
class GameScene : public Engine::Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw(ID3D12GraphicsCommandList* commandList) override;
	void Finalize() override;

private:
	/// <summary>
	/// 斜め視点・平行投影のカメラを設定し、自機の追従を始める
	/// </summary>
	void SetupCamera();

	/// <summary>
	/// カメラが地面上で向いている向き（画面の右・画面の上）を求める。ゲームのカメラ基準で、デバッグカメラは含まない
	/// </summary>
	void GetScreenBasis(Engine::Vector2* outRight, Engine::Vector2* outUp) const;

	/// <summary>
	/// 入力（キーボード・ゲームパッド）から、カメラの向きに合わせた移動方向を求める
	/// </summary>
	[[nodiscard]] Engine::Vector3 ReadMoveDirection(const Engine::Vector2& screenRight, const Engine::Vector2& screenUp) const;

	/// <summary>
	/// 照準を更新する（マウス・右スティックのうち、直近で操作されたほう）。
	/// 狙っている向きと、狙っているか（ゲームパッドで何も傾けていないときは false）を更新する
	/// </summary>
	void UpdateAim(const Engine::Vector2& screenRight, const Engine::Vector2& screenUp);

	/// <summary>
	/// 発射ボタンが押されているか（マウス左・RT・RB。右スティックは、狙うだけで撃たない）
	/// </summary>
	[[nodiscard]] bool IsFireHeld() const;

private:
	Shooter::Arena arena_;
	Shooter::Player player_;
	Shooter::BulletManager bullets_;
	Shooter::EnemyManager enemies_;
	Engine::Light light_;
	std::unique_ptr<Engine::Model> reticle_; // 照準（地面に出す円盤）

	// 照準
	Shooter::AimSource aimSource_ = Shooter::AimSource::kMouse;
	Engine::Vector3 aimDirection_ = { 0.0f, 0.0f, 1.0f }; // 直近の、狙っている向き
	Engine::Vector3 aimPoint_ = { 0.0f, 0.0f, 0.0f };     // 照準を表示する地面上の位置
	bool aiming_ = false;                                 // 狙っているか（ゲームパッドで、スティックを離している間は false）
	bool reticleVisible_ = false;

	// 連射
	float fireCooldown_ = 0.0f;

	// 他のシーンのために、終了時に戻すカメラの設定
	Engine::ProjectionType savedProjection_ = Engine::ProjectionType::kPerspective;
	float savedOrthoHeight_ = 10.0f;
	Engine::Vector3 savedTranslate_ = { 0.0f, 0.0f, -10.0f };

	bool showColliders_ = true; // 当たり判定のデバッグ表示（Debug/Developmentのみ有効）
};

#pragma once
#include "../../Engine/Scene/Scene.h"
#include "../../Engine/3D/Camera.h"
#include "../../Engine/Graphics/Light.h"
#include "../Game/Arena.h"
#include "../Game/Player.h"

/// <summary>
/// ゲーム本体のシーン。壁で囲まれたアリーナで、自機を動かす
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
	/// 入力（キーボード・ゲームパッド）から、カメラの向きに合わせた移動方向を求める
	/// </summary>
	[[nodiscard]] Engine::Vector3 ReadMoveDirection() const;

private:
	Shooter::Arena arena_;
	Shooter::Player player_;
	Engine::Light light_;

	// 他のシーンのために、終了時に戻すカメラの設定
	Engine::ProjectionType savedProjection_ = Engine::ProjectionType::kPerspective;
	float savedOrthoHeight_ = 10.0f;
	Engine::Vector3 savedTranslate_ = { 0.0f, 0.0f, -10.0f };

	bool showColliders_ = true; // 当たり判定のデバッグ表示（Debug/Developmentのみ有効）
};

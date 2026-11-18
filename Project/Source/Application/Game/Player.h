#pragma once
#include <memory>
#include "CombatMath.h"
#include "../../Engine/Collision/Collision3D.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Math/Matrix4x4.h"
#include "../../Engine/Math/Vector3.h"

namespace Engine { class DebugDraw; }

namespace Shooter {

class Arena;

/// <summary>
/// 自機。移動（加減速つき）、向き、壁との押し戻し、描画を受け持つ。
/// 位置は足元の座標で、当たり判定は、立っているカプセル
/// </summary>
class Player {
public:
	/// <summary>
	/// 見た目のモデル（本体と、向きを示す鼻）を作る
	/// </summary>
	void Initialize(Engine::AssetFactory* assetFactory, const Engine::Vector3& startPosition);

	/// <summary>
	/// 1フレーム分、動かす
	/// </summary>
	/// <param name="deltaTime">経過時間（秒）</param>
	/// <param name="moveDirection">移動したい向き（ワールドのXZ平面、長さ0〜1。長さが速さの割合になる）</param>
	/// <param name="arena">壁との押し戻しに使う</param>
	/// <param name="aimDirection">狙っている向き（ワールドのXZ平面、長さ1）。長さが0なら、狙っていないので、進む向きを向く</param>
	void Update(float deltaTime, const Engine::Vector3& moveDirection, const Engine::Vector3& aimDirection, const Arena& arena);

	/// <summary>
	/// ダメージを受ける。無敵中・すでに倒れている場合は、何もしない。受けたら、一定時間の無敵になる
	/// </summary>
	/// <returns>ダメージを受けたら true</returns>
	bool TakeDamage(int amount);

	/// <summary>
	/// 体力が尽きているか（倒れている間は、操作を受け付けない）
	/// </summary>
	[[nodiscard]] bool IsDead() const { return Shooter::IsDead(health_); }

	[[nodiscard]] const Health& GetHealth() const { return health_; }

	/// <summary>
	/// 描画する。無敵の間は、点滅する
	/// </summary>
	void Draw(const Engine::Matrix4x4& viewProjection);

	/// <summary>
	/// 当たり判定のカプセル
	/// </summary>
	[[nodiscard]] Engine::Capsule GetCapsule() const;

	/// <summary>
	/// 足元の座標（カメラの追従対象のポインタとしても使えるよう、参照で返す）
	/// </summary>
	[[nodiscard]] const Engine::Vector3& GetPosition() const { return position_; }

	[[nodiscard]] const Engine::Vector3& GetVelocity() const { return velocity_; }

	/// <summary>
	/// 向き（Y軸まわりの角度。+Zが0）
	/// </summary>
	[[nodiscard]] float GetYaw() const { return yaw_; }

	/// <summary>
	/// 当たり判定を、デバッグ表示に追加する
	/// </summary>
	void DrawDebug(Engine::DebugDraw& debugDraw) const;

private:
	/// <summary>
	/// モデルの位置・向きを、現在の状態に合わせる
	/// </summary>
	void SyncModels();

private:
	Engine::Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	Engine::Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };
	float yaw_ = 0.0f;

	std::unique_ptr<Engine::Model> body_;
	std::unique_ptr<Engine::Model> nose_;

	Health health_;
};

} // namespace Shooter

#include "Player.h"
#include "Arena.h"
#include "PlayerMath.h"
#include "ShooterConfig.h"
#include "../../Engine/DebugTools/DebugDraw.h"
#include <cmath>

using namespace Engine;

namespace Shooter {

void Player::Initialize(AssetFactory* assetFactory, const Vector3& startPosition) {
	position_ = startPosition;
	velocity_ = Vector3{ 0.0f, 0.0f, 0.0f };
	yaw_ = 0.0f;

	// 本体: 判定のカプセルと同じ大きさの円柱
	body_ = assetFactory->CreateModelInstance(assetFactory->CreatePrimitiveModelData(PrimitiveType::kCylinder));
	body_->GetScale() = Vector3{ Config::kPlayerRadius * 2.0f, Config::kPlayerHeight, Config::kPlayerRadius * 2.0f };
	body_->SetColor(Vector4{ 0.3f, 0.6f, 1.0f, 1.0f });

	// 鼻: 向いている方向が分かるように、前に突き出す小さな箱
	nose_ = assetFactory->CreateModelInstance(assetFactory->CreatePrimitiveModelData(PrimitiveType::kBox));
	nose_->GetScale() = Vector3{ 0.3f, 0.3f, 0.6f };
	nose_->SetColor(Vector4{ 1.0f, 0.85f, 0.2f, 1.0f });

	SyncModels();
}

void Player::Update(float deltaTime, const Vector3& moveDirection, const Arena& arena) {
	// 目標の速度へ、加速（入力をやめたときは、強めに減速）して近づく
	const Vector3 targetVelocity = moveDirection * Config::kPlayerSpeed;
	const bool hasInput = Length(moveDirection) > 1e-4f;
	const float rate = hasInput ? Config::kPlayerAcceleration : Config::kPlayerDeceleration;
	velocity_ = ApproachVelocity(velocity_, targetVelocity, rate * deltaTime);

	position_ += velocity_ * deltaTime;

	// 壁にめり込んだら、押し戻す。壁に向かう速度の成分は、打ち消す（壁に張り付いて、滑るように動く）
	Capsule capsule = GetCapsule();
	const Vector3 correction = arena.ResolveCapsule(capsule);
	if (Dot(correction, correction) > 0.0f) {
		position_ += correction;
		const Vector3 pushDirection = Normalize(correction);
		const float intoWall = Dot(velocity_, pushDirection);
		if (intoWall < 0.0f) {
			velocity_ -= pushDirection * intoWall;
		}
	}
	position_.y = 0.0f;

	// 向き: 動いている間は、進む向きへ、滑らかに回る
	if (hasInput && Length(velocity_) > 0.5f) {
		const float targetYaw = YawFromDirection(velocity_, yaw_);
		yaw_ = RotateTowardsAngle(yaw_, targetYaw, Config::kPlayerTurnSpeed * deltaTime);
	}

	SyncModels();
}

void Player::SyncModels() {
	body_->GetTranslate() = Vector3{ position_.x, position_.y + Config::kPlayerHeight * 0.5f, position_.z };

	const Vector3 forward = Vector3{ std::sin(yaw_), 0.0f, std::cos(yaw_) };
	nose_->GetTranslate() = position_ + forward * (Config::kPlayerRadius + 0.15f) + Vector3{ 0.0f, Config::kPlayerHeight * 0.65f, 0.0f };
	nose_->GetRotate() = Vector3{ 0.0f, yaw_, 0.0f };
}

void Player::Draw(const Matrix4x4& viewProjection) {
	body_->Draw(viewProjection);
	nose_->Draw(viewProjection);
}

Capsule Player::GetCapsule() const {
	return MakeUprightCapsule(position_, Config::kPlayerHeight, Config::kPlayerRadius);
}

void Player::DrawDebug(DebugDraw& debugDraw) const {
	debugDraw.Draw(GetCapsule(), DebugColor::kGreen);
	// 速度の向き
	if (Length(velocity_) > 0.1f) {
		debugDraw.Arrow(position_ + Vector3{ 0.0f, 0.1f, 0.0f }, position_ + Vector3{ 0.0f, 0.1f, 0.0f } + velocity_ * 0.3f, DebugColor::kCyan, 0.2f);
	}
}

} // namespace Shooter

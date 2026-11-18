#pragma once
#include <array>
#include <memory>
#include "../../Engine/Collision/Collision3D.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Graphics/TextureManager.h"
#include "../../Engine/Math/Matrix4x4.h"
#include "../../Engine/Math/Vector2.h"

namespace Engine { class DebugDraw; }

namespace Shooter {

/// <summary>
/// 壁で囲まれた、固定の戦場。地面と4枚の壁（見た目と判定）を持つ
/// </summary>
class Arena {
public:
	/// <summary>
	/// 地面と壁のモデルを作る
	/// </summary>
	void Initialize(Engine::AssetFactory* assetFactory);

	/// <summary>
	/// 地面と壁を描画する
	/// </summary>
	void Draw(const Engine::Matrix4x4& viewProjection);

	/// <summary>
	/// カプセルが壁にめり込んでいたら、押し戻す（角でも抜けないよう、数回繰り返す）。
	/// 押し戻した量（水平方向）を返す
	/// </summary>
	Engine::Vector3 ResolveCapsule(Engine::Capsule& capsule) const;

	/// <summary>
	/// 中心から壁の内側までの長さ（X, Z）
	/// </summary>
	[[nodiscard]] Engine::Vector2 GetHalfExtent() const;

	/// <summary>
	/// 壁の判定（AABB）
	/// </summary>
	[[nodiscard]] const std::array<Engine::AABB, 4>& GetWalls() const { return walls_; }

	/// <summary>
	/// 壁の判定を、デバッグ表示に追加する
	/// </summary>
	void DrawDebug(Engine::DebugDraw& debugDraw) const;

private:
	static constexpr int kWallCount = 4;

	std::array<Engine::AABB, 4> walls_ = {};
	std::unique_ptr<Engine::Model> ground_;
	std::array<std::unique_ptr<Engine::Model>, kWallCount> wallModels_;
	Engine::TextureHandle groundTexture_; // 地面の模様（Modelはテクスチャを所有しないので、ここで保持する）
};

} // namespace Shooter

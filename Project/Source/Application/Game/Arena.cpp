#include "Arena.h"
#include "ShooterConfig.h"
#include "../../Engine/DebugTools/DebugDraw.h"
#include "../../Engine/Math/TransformMatrix.h"

using namespace Engine;

namespace Shooter {

void Arena::Initialize(AssetFactory* assetFactory) {
	const float halfW = Config::kArenaHalfWidth;
	const float halfD = Config::kArenaHalfDepth;
	const float thick = Config::kWallThickness;
	const float height = Config::kWallHeight;

	// 壁の判定（AABB）。アリーナの内側の外周に、厚みを持たせて4枚置く（角で隙間ができないよう、左右の壁を長くする）
	walls_[0] = AABB{ Vector3{ -halfW - thick, 0.0f, -halfD - thick }, Vector3{ halfW + thick, height, -halfD } }; // 手前
	walls_[1] = AABB{ Vector3{ -halfW - thick, 0.0f, halfD }, Vector3{ halfW + thick, height, halfD + thick } };   // 奥
	walls_[2] = AABB{ Vector3{ -halfW - thick, 0.0f, -halfD }, Vector3{ -halfW, height, halfD } };                 // 左
	walls_[3] = AABB{ Vector3{ halfW, 0.0f, -halfD }, Vector3{ halfW + thick, height, halfD } };                   // 右

	// 地面: 水平な板を、アリーナの大きさに広げ、目立たない格子の画像を繰り返して貼る（動いていることが分かりやすいように）
	constexpr float kTileSize = 4.0f;
	groundTexture_ = assetFactory->LoadTexture("Resource/ground.png");
	ground_ = assetFactory->CreateModelInstance(assetFactory->CreatePrimitiveModelData(PrimitiveType::kPlane));
	ground_->GetScale() = Vector3{ halfW * 2.0f, 1.0f, halfD * 2.0f };
	ground_->SetTextureHandle(groundTexture_.gpuHandle);
	ground_->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
	// 1枚の画像が、kTileSize の長さに対応するよう、UVを拡大する
	ground_->GetMaterial().GetUVTransform() = MakeScaleMatrix(Vector3{ halfW * 2.0f / kTileSize, halfD * 2.0f / kTileSize, 1.0f });

	// 壁の見た目: 判定の大きさに合わせた箱
	auto boxData = assetFactory->CreatePrimitiveModelData(PrimitiveType::kBox);
	for (int i = 0; i < kWallCount; ++i) {
		wallModels_[i] = assetFactory->CreateModelInstance(boxData);
		const Vector3 center = (walls_[i].min + walls_[i].max) * 0.5f;
		const Vector3 size = walls_[i].max - walls_[i].min;
		wallModels_[i]->GetTranslate() = center;
		wallModels_[i]->GetScale() = size;
		wallModels_[i]->SetColor(Vector4{ 0.55f, 0.35f, 0.3f, 1.0f });
	}
}

void Arena::Draw(const Matrix4x4& viewProjection) {
	ground_->Draw(viewProjection);
	for (auto& wall : wallModels_) {
		wall->Draw(viewProjection);
	}
}

Vector3 Arena::ResolveCapsule(Capsule& capsule) const {
	Vector3 total = { 0.0f, 0.0f, 0.0f };
	// 1枚の壁から押し戻されたら、別の壁（角）にめり込むことがあるので、数回繰り返す
	for (int iteration = 0; iteration < 4; ++iteration) {
		bool pushed = false;
		for (const AABB& wall : walls_) {
			Contact3D contact = Collide(capsule, wall);
			if (!contact.hit) {
				continue;
			}
			// 水平方向だけ押し戻す（壁は垂直なので、上下には動かさない）
			Vector3 push = Vector3{ contact.normal.x, 0.0f, contact.normal.z } * contact.depth;
			capsule.segment.start += push;
			capsule.segment.end += push;
			total += push;
			pushed = true;
		}
		if (!pushed) {
			break;
		}
	}
	return total;
}

Vector2 Arena::GetHalfExtent() const {
	return Vector2{ Config::kArenaHalfWidth, Config::kArenaHalfDepth };
}

void Arena::DrawDebug(DebugDraw& debugDraw) const {
	for (const AABB& wall : walls_) {
		debugDraw.Draw(wall, DebugColor::kGray);
	}
}

} // namespace Shooter

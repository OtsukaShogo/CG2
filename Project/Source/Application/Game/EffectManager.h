#pragma once
#include <d3d12.h>
#include <memory>
#include "FeedbackEvent.h"
#include "../../Engine/3D/ParticleEffectSystem.h"
#include "../../Engine/Graphics/AssetFactory.h"
#include "../../Engine/Graphics/TextureManager.h"

namespace Engine {
class ConstantBufferAllocator;
class Framework;
}

namespace Shooter {

/// <summary>
/// ゲームの演出（パーティクル）を出して、描画する。出来事（FeedbackEvent）を受け取り、種類に応じた粒を出す
/// </summary>
class EffectManager {
public:
	/// <summary>
	/// パーティクルの描画に必要なもの（粒の形・テクスチャ・GPU のバッファ）を用意する
	/// </summary>
	void Initialize(Engine::Framework* framework);

	/// <summary>
	/// 出来事1件に対する演出を出す（演出がない出来事は、何もしない）
	/// </summary>
	void Handle(const FeedbackEvent& event);

	/// <summary>
	/// 粒を進めて、描画用のデータを更新する。カメラが動いたあと（そのフレームの、最後）に呼ぶ
	/// </summary>
	void Update(Engine::Framework* framework, float deltaTime);

	/// <summary>
	/// 粒を描画する（加算ブレンド。不透明なモデルを描いたあとに呼ぶ）
	/// </summary>
	/// <param name="lightAddress">平行光源の定数バッファのアドレス（パイプラインの切り替えで失われるので、渡し直す）</param>
	void Draw(Engine::Framework* framework, ID3D12GraphicsCommandList* commandList, Engine::ConstantBufferAllocator& constantBuffers, D3D12_GPU_VIRTUAL_ADDRESS lightAddress);

	/// <summary>
	/// すべての粒を消す
	/// </summary>
	void Clear() { particles_.Clear(); }

	[[nodiscard]] size_t GetActiveCount() const { return particles_.GetActiveCount(); }

private:
	Engine::ParticleEffectSystem particles_;
	Engine::TextureHandle texture_;                   // 粒に貼る、丸いぼかしのテクスチャ（Modelと同じく、描画側は所有しないので、ここで保持する）
	std::shared_ptr<Engine::ModelData> quad_;         // 粒の形（板）
};

} // namespace Shooter

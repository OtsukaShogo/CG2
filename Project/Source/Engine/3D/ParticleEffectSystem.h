#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>
#include "ParticleEffectMath.h"
#include "../Core/FrameContext.h"
#include "../Graphics/Material.h"
#include "../Graphics/TextureManager.h"
#include "../Math/Matrix4x4.h"

namespace Engine {

class ConstantBufferAllocator;
class ModelData;

/// <summary>
/// 演出用の粒（火花・煙・軌跡など）を、まとめて管理して描画するクラス。
/// 何個でも、いつでも出せる（Burst で一度にまとめて、Spawn で1つずつ）。常にカメラの方を向く板で描く（ビルボード）。
/// 描画は、1回のインスタンシングで行う。描画前に、Particle用のパイプライン（ブレンドモードは、加算が火花向き）をセットすること
/// </summary>
class ParticleEffectSystem {
public:
	ParticleEffectSystem() = default;
	~ParticleEffectSystem();

	ParticleEffectSystem(const ParticleEffectSystem&) = delete;
	ParticleEffectSystem& operator=(const ParticleEffectSystem&) = delete;

	/// <summary>
	/// 粒の描画に使う GPU リソース（インスタンシング用バッファ・SRV）を作る
	/// </summary>
	/// <param name="capacity">同時に存在できる粒の最大数（超えた分は、出さない）</param>
	void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, uint32_t descriptorSizeSRV, uint32_t capacity);

	/// <summary>
	/// 粒の形（正方形の板。大きさ2の plane.obj を想定）を設定する
	/// </summary>
	void SetModelData(std::shared_ptr<const ModelData> modelData) { modelData_ = std::move(modelData); }

	/// <summary>
	/// 粒に貼るテクスチャ（丸いぼかしなど）を設定する
	/// </summary>
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { material_.SetTextureHandle(handle); }

	/// <summary>
	/// 粒を1つ出す。容量がいっぱいなら、何もしない
	/// </summary>
	void Spawn(const EffectParticle& particle);

	/// <summary>
	/// 設定に従って、粒をまとめて出す（爆発・火花・被弾）。容量を超える分は、出さない
	/// </summary>
	void EmitBurst(const EffectBurst& burst, const Vector3& origin);

	/// <summary>
	/// 粒を進め（寿命の尽きたものは消す）、描画用のデータをバッファへ書き込む
	/// </summary>
	/// <param name="viewProjection">ビュープロジェクション行列</param>
	/// <param name="cameraRotate">カメラのワールド回転行列（ビルボード用）</param>
	/// <param name="frameIndex">今のフレームスロット（FrameContext::frameIndex）</param>
	void Update(const Matrix4x4& viewProjection, const Matrix4x4& cameraRotate, float deltaTime, uint32_t frameIndex);

	/// <summary>
	/// 粒を描画する（1回のインスタンシング）。粒がなければ、何もしない
	/// </summary>
	void Draw(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers);

	/// <summary>
	/// すべての粒を消す（シーンの切り替え時など）
	/// </summary>
	void Clear() { particles_.clear(); drawCount_ = 0; }

	/// <summary>
	/// 存在する粒の数
	/// </summary>
	[[nodiscard]] size_t GetActiveCount() const { return particles_.size(); }

	/// <summary>
	/// 同時に存在できる粒の最大数
	/// </summary>
	[[nodiscard]] uint32_t GetCapacity() const { return capacity_; }

private:
	uint32_t capacity_ = 0;
	std::vector<EffectParticle> particles_;
	std::mt19937 random_{ std::random_device{}() };

	// 描画用: フレームスロットごとの範囲を持つ、1つのバッファと、スロットごとのSRV
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	struct ParticleForGPU* instancingData_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU_[kFrameCount] = {};
	SrvSlot srvSlot_[kFrameCount];
	uint32_t currentFrame_ = 0;
	uint32_t drawCount_ = 0;

	Material material_;
	std::shared_ptr<const ModelData> modelData_;
};

} // namespace Engine

#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <memory>
#include "TransformMatrix.h"
#include "WorldTransform.h"
#include "Material.h"

namespace Engine {

class ModelData;

struct Particle {
	WorldTransform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

/// <summary>
/// インスタンシング用StructuredBufferに渡す1パーティクル分のデータ（Particle.hlsliのParticleForGPUと対応）
/// </summary>
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4   color;
};

/// <summary>
/// インスタンシング描画によるパーティクル群を表すクラス
/// </summary>
class ParticleSystem {
public:
	// インスタンス数
	static constexpr uint32_t kNumMaxInstance = 10;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	ParticleSystem();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ParticleSystem();

	/// <summary>
	/// インスタンシング用のTransformationMatrixリソースを生成し、単位行列で初期化した上でSRVを作成する
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="srvHeap">SRV用ディスクリプタヒープ</param>
	/// <param name="descriptorSizeSRV">SRVディスクリプタ1個分のサイズ</param>
	void CreateInstancingResource(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, uint32_t descriptorSizeSRV);

	/// <summary>
	/// インスタンシング用SRVのGPUディスクリプタハンドルを取得する
	/// </summary>
	/// <returns>GPUディスクリプタハンドル</returns>
	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetInstancingSrvHandleGPU() const { return instancingSrvHandleGPU_; }

	/// <summary>
	/// 各インスタンスのワールド行列・WVP行列を計算し、インスタンシング用リソースへ書き込む
	/// </summary>
	/// <param name="viewProjection">適用するビュープロジェクション行列</param>
	/// <param name="cameraRotate">カメラのワールド回転行列（ビルボード用）</param>
	/// <param name="deltaTime">経過時間（秒）。0を渡すと寿命が進まない</param>
	void Update(const Matrix4x4& viewProjection, const Matrix4x4& cameraRotate, float deltaTime);

	/// <summary>
	/// 描画に使うメッシュデータ（共有描画データ）を設定する
	/// </summary>
	/// <param name="modelData">参照するModelData</param>
	void SetModelData(std::shared_ptr<const ModelData> modelData) { modelData_ = std::move(modelData); }

	/// <summary>
	/// テクスチャのGPUディスクリプタハンドルを設定する
	/// </summary>
	/// <param name="handle">設定するGPUディスクリプタハンドル</param>
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { material_.SetTextureHandle(handle); }

	/// <summary>
	/// マテリアルの色を取得する（変更可能）
	/// </summary>
	/// <returns>マテリアルの色</returns>
	Vector4& GetColor() { return material_.GetColor(); }

	/// <summary>
	/// 指定インデックスのパーティクルを取得する（変更可能）
	/// </summary>
	/// <param name="index">パーティクルのインデックス（0 ～ kNumInstance-1）</param>
	/// <returns>パーティクル</returns>
	[[nodiscard]] Particle& GetParticle(uint32_t index) { return particles_[index]; }

	/// <summary>
	/// 指定インデックスのパーティクルを取得する
	/// </summary>
	/// <param name="index">パーティクルのインデックス（0 ～ kNumInstance-1）</param>
	/// <returns>パーティクル</returns>
	[[nodiscard]] const Particle& GetParticle(uint32_t index) const { return particles_[index]; }

	/// <summary>
	/// 指定インデックスのパーティクルを設定する
	/// </summary>
	/// <param name="index">パーティクルのインデックス（0 ～ kNumInstance-1）</param>
	/// <param name="particle">設定するパーティクル</param>
	void SetParticle(uint32_t index, const Particle& particle) { particles_[index] = particle; }

	/// <summary>
	/// パーティクルをインスタンシング描画する
	/// </summary>
	/// <param name="commandList">描画コマンドを積むコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 今あるパーティクルを破棄し、ランダムな位置・速度・色で生成し直す
	/// </summary>
	void Respawn();

	/// <summary>
	/// ビルボード（常にカメラへ向ける）の有効/無効フラグを取得する（変更可能）
	/// </summary>
	/// <returns>ビルボード有効フラグ</returns>
	bool& GetUseBillboard() { return useBillboard_; }

private:
	/// <summary>
	/// 全パーティクルをランダムな位置・速度・色で初期化する
	/// </summary>
	void InitializeParticles();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	ParticleForGPU* instancingData_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};
	uint32_t numInstance = 0;
	bool useBillboard_ = true;
	Particle particles_[kNumMaxInstance]{};

	Material material_;
	std::shared_ptr<const ModelData> modelData_;
};

} // namespace Engine

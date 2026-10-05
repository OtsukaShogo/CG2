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

/// <summary>
/// インスタンシング描画によるパーティクル群を表すクラス
/// </summary>
class Particle {
public:
	// インスタンス数
	static constexpr uint32_t kNumInstance = 10;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Particle();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Particle();

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
	void Update(const Matrix4x4& viewProjection);

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
	/// パーティクルをインスタンシング描画する
	/// </summary>
	/// <param name="commandList">描画コマンドを積むコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	TransformationMatrix* instancingData_ = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};
	WorldTransform transforms_[kNumInstance]{};

	Material material_;
	std::shared_ptr<const ModelData> modelData_;
};

} // namespace Engine

#include "Particle.h"
#include "D3D12Util.h"
#include "Matrix4x4.h"
#include "TextureManager.h"
#include "ModelData.h"
#include "PipelineStateManager.h"

namespace Engine {

Particle::Particle() {}

Particle::~Particle() {
	if (instancingResource_) {
		instancingResource_->Unmap(0, nullptr);
	}
}

// Instancing用のTransformationMatrixリソースを作り、単位行列を書き込んだ上でSRVを作成する
void Particle::CreateInstancingResource(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, uint32_t descriptorSizeSRV) {
	instancingResource_ = CreateBufferResource(device, sizeof(TransformationMatrix) * kNumInstance);

	// 書き込むためのアドレスを取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		instancingData_[index].WVP = MakeIdentity4x4();
		instancingData_[index].World = MakeIdentity4x4();
	}

	// 各インスタンスの位置が少しずつずれるように初期化する
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		transforms_[index].scale = { 1.0f, 1.0f, 1.0f };
		transforms_[index].rotate = { 0.0f, 0.0f, 0.0f };
		transforms_[index].translate = { index * 0.1f, index * 0.1f, index * 0.1f };
	}

	// SRV の空きスロットを自動割り当て（テクスチャと同じヒープ・同じカウンタを共有する）
	uint32_t index = TextureManager::GetInstance()->AllocateSrvIndex();

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = GetCPUDescriptorHandle(srvHeap, descriptorSizeSRV, index);
	instancingSrvHandleGPU_ = GetGPUDescriptorHandle(srvHeap, descriptorSizeSRV, index);

	// StructuredBuffer として参照するためのSRVを作成する
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;                        // 構造体のレイアウトは自由なので不明
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;          // TextureではなくBufferとして利用する
	instancingSrvDesc.Buffer.FirstElement = 0;                            // 当面0
	instancingSrvDesc.Buffer.NumElements = kNumInstance;                  // 10個すべてにアクセスする
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix); // Alignmentルールはsizeofのままで良い
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	// マテリアル（色・テクスチャ）用の定数バッファを生成する。パーティクルはライティングしない
	material_.Create(device);
	material_.SetEnableLighting(false);
}

// 各インスタンスのワールド行列・WVP行列を計算し、インスタンシング用リソースへ書き込む
void Particle::Update(const Matrix4x4& viewProjection) {
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		Matrix4x4 worldMatrix = MakeAffineMatrix(transforms_[index].scale, transforms_[index].rotate, transforms_[index].translate);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, viewProjection);
		instancingData_[index].WVP = worldViewProjectionMatrix;
		instancingData_[index].World = worldMatrix;
	}
}

// インスタンシング用SRV・マテリアルをバインドし、インスタンス数分まとめて描画する
void Particle::Draw(ID3D12GraphicsCommandList* commandList) {
	commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kInstancing), instancingSrvHandleGPU_);
	material_.Bind(commandList);
	modelData_->GetMesh().Draw(commandList, kNumInstance);
}

} // namespace Engine

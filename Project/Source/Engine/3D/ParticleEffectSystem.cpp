#include "ParticleEffectSystem.h"
#include "ModelData.h"
#include "ParticleSystem.h" // ParticleForGPU（炎のパーティクルと、同じシェーダーを使うので、同じ構造）
#include "../Core/D3D12Util.h"
#include "../Core/PipelineStateManager.h"
#include "../Graphics/ConstantBufferAllocator.h"
#include "../Math/Matrix4x4.h"
#include "../Math/TransformMatrix.h"
#include <algorithm>
#include <numbers>

namespace Engine {

ParticleEffectSystem::~ParticleEffectSystem() {
	if (instancingResource_) {
		instancingResource_->Unmap(0, nullptr);
	}
}

void ParticleEffectSystem::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, uint32_t descriptorSizeSRV, uint32_t capacity) {
	capacity_ = capacity;
	particles_.clear();
	particles_.reserve(capacity);

	// GPUが前のフレームの範囲を読んでいる間もCPUが書き込めるよう、フレームスロット分の範囲を1つのバッファに確保する
	const uint32_t totalInstances = capacity * kFrameCount;
	instancingResource_ = CreateBufferResource(device, sizeof(ParticleForGPU) * totalInstances);
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	for (uint32_t i = 0; i < totalInstances; ++i) {
		instancingData_[i].WVP = MakeIdentity4x4();
		instancingData_[i].World = MakeIdentity4x4();
		instancingData_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	for (uint32_t frame = 0; frame < kFrameCount; ++frame) {
		srvSlot_[frame] = TextureManager::GetInstance()->AllocateSrv();
		const uint32_t index = *srvSlot_[frame];

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUDescriptorHandle(srvHeap, descriptorSizeSRV, index);
		srvHandleGPU_[frame] = GetGPUDescriptorHandle(srvHeap, descriptorSizeSRV, index);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = static_cast<UINT64>(frame) * capacity;
		desc.Buffer.NumElements = capacity;
		desc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
		desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		device->CreateShaderResourceView(instancingResource_.Get(), &desc, cpuHandle);
	}

	material_.SetEnableLighting(false); // 粒は、光を受けない
}

void ParticleEffectSystem::Spawn(const EffectParticle& particle) {
	if (particles_.size() >= capacity_) {
		return;
	}
	particles_.push_back(particle);
}

void ParticleEffectSystem::EmitBurst(const EffectBurst& burst, const Vector3& origin) {
	// 容量を超える分は出さない（一度に作ってから、入る分だけ追加する）
	std::vector<EffectParticle> created;
	GenerateBurst(burst, origin, random_, created);
	const size_t room = capacity_ > particles_.size() ? capacity_ - particles_.size() : 0;
	const size_t count = (std::min)(room, created.size());
	particles_.insert(particles_.end(), created.begin(), created.begin() + static_cast<std::ptrdiff_t>(count));
}

void ParticleEffectSystem::Update(const Matrix4x4& viewProjection, const Matrix4x4& cameraRotate, float deltaTime, uint32_t frameIndex) {
	currentFrame_ = frameIndex;
	ParticleForGPU* instancing = instancingData_ + static_cast<size_t>(frameIndex) * capacity_; // このフレームスロットの範囲

	// 板を、常にカメラの方へ向ける（板の裏表を反転させた上で、カメラの回転に追従させる）
	Matrix4x4 billboard = Multiply(MakeRotateYMatrix(std::numbers::pi_v<float>), cameraRotate);
	billboard.m[3][0] = 0.0f; // 平行移動成分は使わない
	billboard.m[3][1] = 0.0f;
	billboard.m[3][2] = 0.0f;

	// 進めて、寿命の尽きたものを消す
	particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
		[deltaTime](EffectParticle& p) { return !StepParticle(p, deltaTime); }), particles_.end());

	drawCount_ = 0;
	for (const EffectParticle& particle : particles_) {
		if (drawCount_ >= capacity_) {
			break;
		}
		// 板（大きさ2）を、粒の大きさ（ワールドの長さ）に合わせて拡大する
		const float half = (std::max)(0.0f, ParticleSize(particle)) * 0.5f;
		const Matrix4x4 world = Multiply(Multiply(MakeScaleMatrix(Vector3{ half, half, half }), billboard), MakeTranslateMatrix(particle.position));
		instancing[drawCount_].WVP = Multiply(world, viewProjection);
		instancing[drawCount_].World = world;
		instancing[drawCount_].color = ParticleColor(particle);
		++drawCount_;
	}
}

void ParticleEffectSystem::Draw(ID3D12GraphicsCommandList* commandList, ConstantBufferAllocator& constantBuffers) {
	// 粒が0個のDrawは、デバッグレイヤーで警告になるので、描かない
	if (drawCount_ == 0 || !modelData_) {
		return;
	}
	commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kInstancing), srvHandleGPU_[currentFrame_]);
	material_.Bind(commandList, constantBuffers);
	modelData_->GetMesh().Draw(commandList, drawCount_);
}

} // namespace Engine

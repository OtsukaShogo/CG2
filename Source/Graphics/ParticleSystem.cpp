#include "ParticleSystem.h"
#include "D3D12Util.h"
#include "Matrix4x4.h"
#include "TextureManager.h"
#include "ModelData.h"
#include "PipelineStateManager.h"
#include "Random.h"
#include <algorithm>
#include <numbers>
#include <random>

namespace Engine {

namespace {

/// <summary>
/// EaseInCubic。序盤はゆっくり、終盤に向けて加速する
/// </summary>
/// <param name="t">進行度（0.0～1.0）</param>
/// <returns>イージング後の進行度（0.0～1.0）</returns>
float EaseInCubic(float t) {
	return t * t * t;
}

/// <summary>
/// ランダムな位置・速度を持つパーティクルを1つ生成する
/// </summary>
/// <param name="randomEngine">乱数エンジン</param>
/// <returns>生成したパーティクル</returns>
Particle MakeNewParticle(std::mt19937& randomEngine) {
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	Particle particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine) ,distColor(randomEngine) ,1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0.0f;

	return particle;
}

/// <summary>
/// エミッタの設定に従ってパーティクルを emitter.count 個生成する。生成位置はエミッタ位置が基準になる
/// </summary>
/// <param name="emitter">エミッタ</param>
/// <param name="randomEngine">乱数エンジン</param>
/// <returns>生成したパーティクルのリスト</returns>
std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particle> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		Particle particle = MakeNewParticle(randomEngine);
		particle.transform.translate += emitter.transform.translate;
		particles.push_back(particle);
	}
	return particles;
}

} // namespace

ParticleSystem::ParticleSystem() {
	// 一度に3個発生させるエミッタ
	emitter_.transform.scale = { 1.0f, 1.0f, 1.0f };
	emitter_.transform.rotate = { 0.0f, 0.0f, 0.0f };
	emitter_.transform.translate = { 0.0f, 0.0f, 0.0f };
	emitter_.count = 3;
	emitter_.frequency = 0.5f;
	emitter_.frequencyTime = 0.0f;
}

ParticleSystem::~ParticleSystem() {
	if (instancingResource_) {
		instancingResource_->Unmap(0, nullptr);
	}
}

// Instancing用のParticleForGPUリソースを作り、単位行列を書き込んだ上でSRVを作成する
void ParticleSystem::CreateInstancingResource(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, uint32_t descriptorSizeSRV) {
	instancingResource_ = CreateBufferResource(device, sizeof(ParticleForGPU) * kNumMaxInstance);

	// 書き込むためのアドレスを取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		instancingData_[index].WVP = MakeIdentity4x4();
		instancingData_[index].World = MakeIdentity4x4();
		instancingData_[index].color = { 1.0f, 1.0f, 1.0f, 1.0f };
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
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;                  // 10個すべてにアクセスする
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU); // Alignmentルールはsizeofのままで良い
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	// マテリアル（色・テクスチャ）用の定数バッファを生成する。パーティクルはライティングしない
	material_.Create(device);
	material_.SetEnableLighting(false);
}

// 各インスタンスのワールド行列・WVP行列を計算し、インスタンシング用リソースへ書き込む
void ParticleSystem::Update(const Matrix4x4& viewProjection, const Matrix4x4& cameraRotate, float deltaTime) {
	// 板ポリの裏表を反転させた上でカメラの回転に追従させる（ビルボード）
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	// ビルボード無効時はカメラ回転を掛けず、板ポリを元の向き（-Z側から見て表）のままにする
	Matrix4x4 billboardMatrix = useBillboard_ ? Multiply(backToFrontMatrix, cameraRotate) : backToFrontMatrix;
	billboardMatrix.m[3][0] = 0.0f; // 平行移動成分は使わない
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	numInstance = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin(); particleIterator != particles_.end();) {
		//生存時間を過ぎていたらリストから削除する（erase は次の要素のイテレータを返す）
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles_.erase(particleIterator);
			continue;
		}
		// 速度を反映して移動し、経過時間を進める
		(*particleIterator).transform.translate += (*particleIterator).velocity * deltaTime;
		(*particleIterator).currentTime += deltaTime;

		//徐々に透明に（イージングで序盤は残り、終盤に急に消える）
		float t = std::clamp((*particleIterator).currentTime / (*particleIterator).lifeTime, 0.0f, 1.0f);
		float alpha = 1.0f - EaseInCubic(t);

		if (numInstance < kNumMaxInstance) {
			Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
			Matrix4x4 worldMatrix = Multiply(Multiply(scaleMatrix, billboardMatrix), translateMatrix);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, viewProjection);
			// 死んだものを詰めて、先頭から numInstance 個を描画できるようにする
			instancingData_[numInstance].WVP = worldViewProjectionMatrix;
			instancingData_[numInstance].World = worldMatrix;
			instancingData_[numInstance].color = (*particleIterator).color;
			instancingData_[numInstance].color.w = alpha;
			++numInstance;
		}
		++particleIterator;

		// インスタンシング用バッファの容量を超える分は描画しない
		if (numInstance >= kNumMaxInstance) {
			break;
		}
	}
}

// インスタンシング用SRV・マテリアルをバインドし、インスタンス数分まとめて描画する
void ParticleSystem::Draw(ID3D12GraphicsCommandList* commandList) {
	// 全パーティクルが寿命を迎えた場合、InstanceCount=0 のDrawはデバッグレイヤーでブレークするため描画しない
	if (numInstance == 0) {
		return;
	}
	commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kInstancing), instancingSrvHandleGPU_);
	material_.Bind(commandList);
	modelData_->GetMesh().Draw(commandList, numInstance);
}

// エミッタの設定に従って、パーティクルを emitter.count 個生成する
void ParticleSystem::EmitParticles() {
	particles_.splice(particles_.end(), Emit(emitter_, Random::GetInstance()->GetEngine()));
}

} // namespace Engine

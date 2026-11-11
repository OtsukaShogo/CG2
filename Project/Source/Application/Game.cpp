#include "Game.h"

#include "../Engine/Core/ImGuiManager.h"
#include "../Engine/3D/CameraManager.h"
#include "../Engine/Math/TransformMatrix.h"

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

using namespace Engine;

void Game::OnInitialize() {
	AssetFactory* assetFactory = GetAssetFactory();
	DirectXCommon* dx = GetDirectX();
	ID3D12Device* device = dx->GetDevice();

	// === テクスチャ ===
	tex_ = assetFactory->LoadTexture("Resource/uvChecker.png");
	particleTex_ = assetFactory->LoadTexture("Resource/circle.png");

	// === 音声 ===
	AudioManager* audio = GetAudio();
	soundData1_ = audio->LoadWave("Resource/Alarm01.wav");
	audio->Play(soundData1_);

	// パーティクルの描画形状（板）を読み込む
	particleModelData_ = assetFactory->CreateModelData("Resource", "plane.obj");

	// === スプライト ===
	sprite_ = assetFactory->CreateSprite(640.0f, 360.0f);
	sprite_->SetTextureHandle(tex_.gpuHandle);

	// === パーティクル ===
	particle_ = std::make_unique<ParticleSystem>();
	particle_->CreateInstancingResource(device, dx->GetSrvDescriptorHeap(), dx->GetDescriptorSizeSRV());
	particle_->SetModelData(particleModelData_);
	particle_->SetTextureHandle(particleTex_.gpuHandle); // circleを使う

	// 風（パーティクル以外のゲームオブジェクトからも参照できる）
	wind_.SetAcceleration({ -6.0f, 5.0f, 0.0f }); // 左上へ流す
	wind_.SetArea({ { -8.0f, -8.0f, -2.0f }, { 8.0f, 8.0f, 2.0f } });
	particle_->SetWind(&wind_);

	// === ライト ===
	light_ = std::make_unique<Light>();
	light_->Initialize(device);
}

void Game::OnUpdate() {
#ifdef USE_IMGUI
	ImGui::Begin("Debug");

	ImGui::Text("%f", ImGui::GetIO().Framerate);

	GetCameraManager()->DrawImGui();
	ImGui::Separator();

	ImGui::Combo("Blend Mode", &currentBlendMode_, kBlendModeNames, static_cast<int>(BlendMode::kCountOfBlendMode));

	//パーティクル
	if (ImGui::Button("Particle Start")) {
		particle_->EmitParticles(); // エミッタ（count=3）に従ってパーティクルを追加する
		isParticleMoving_ = true;
	}
	ImGui::Checkbox("Billboard", &particle_->GetUseBillboard());
	ImGui::DragFloat3("Emitter X", &particle_->GetEmitter().transform.translate.x, 0.1f);

	//風
	ImGui::Checkbox("Wind Enabled", &wind_.GetEnabled());
	ImGui::DragFloat3("Wind Acceleration", &wind_.GetField().acceleration.x, 0.1f);
	ImGui::DragFloat3("Wind Area Min", &wind_.GetField().area.min.x, 0.1f);
	ImGui::DragFloat3("Wind Area Max", &wind_.GetField().area.max.x, 0.1f);

	//ライト
	ImGui::ColorEdit4("Light Color", &light_->GetDirectionalLight()->color.x);
	ImGui::DragFloat3("Light Direction", &light_->GetDirectionalLight()->direction.x, 0.1f);
	ImGui::DragFloat("Light Intensity", &light_->GetDirectionalLight()->intensity, 0.1f);

	ImGui::End();
#endif

	// スプライトUV更新
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite_.translate));
	sprite_->GetMaterial().GetUVTransform() = uvTransformMatrix;

	// パーティクルのWVP等を計算してResourceに書き込む
	CameraManager* cameraMgr = GetCameraManager();
	particle_->Update(cameraMgr->GetViewProjection(), cameraMgr->GetRotateMatrix(),
		isParticleMoving_ ? GetDeltaTime() : 0.0f);
}

void Game::OnDraw(ID3D12GraphicsCommandList* commandList) {
	PipelineStateManager* psoMgr = GetPipelineStateManager();
	BlendMode blendMode = static_cast<BlendMode>(currentBlendMode_);

	// 3D描画（パーティクル）
	psoMgr->SetPipeline(commandList, PipelineStateManager::kParticle, blendMode);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kDirectionalLight), light_->GetDirectionalLightAddress());
	particle_->Draw(commandList);

	// 2D描画
	psoMgr->SetPipeline(commandList, PipelineStateManager::kSprite, blendMode);
	commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kTexture), tex_.gpuHandle);
}

void Game::OnFinalize() {
	light_.reset();             // ~Light() で Unmap + Release
	particle_.reset();          // ID3D12Resource x2 (instancing, material)
	particleModelData_.reset(); // ID3D12Resource x3 (vertex, index, texture)
	sprite_.reset();            // ID3D12Resource x3 (vertex, material, wvp)
	GetAudio()->Unload(&soundData1_);
	tex_ = {};                  // ID3D12Resource x2 (texture, intermediate)
	particleTex_ = {};
}

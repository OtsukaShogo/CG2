#include "DemoScene.h"

#include "../../Engine/Core/Framework.h"
#include "../../Engine/3D/CameraManager.h"
#include "../../Engine/Input/Input.h"
#include "../../Engine/Math/TransformMatrix.h"

#ifdef USE_IMGUI
#include "External/imgui/imgui.h"
#endif

using namespace Engine;

void DemoScene::Initialize() {
	Framework* framework = GetFramework();
	AssetFactory* assetFactory = framework->GetAssetFactory();
	DirectXCommon* dx = framework->GetDirectX();
	ID3D12Device* device = dx->GetDevice();

	// === テクスチャ ===
	tex_ = assetFactory->LoadTexture("Resource/uvChecker.png");
	particleTex_ = assetFactory->LoadTexture("Resource/circle.png");

	// === 音声 ===
	AudioManager* audio = framework->GetAudio();
	soundData1_ = audio->LoadWave("Resource/Alarm01.wav");
	voice1_ = audio->Play(soundData1_);

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
}

void DemoScene::Update() {
	Framework* framework = GetFramework();

#ifdef USE_IMGUI
	ImGui::Begin("Debug");

	ImGui::Text("%f", ImGui::GetIO().Framerate);

	// シーンの作り直し
	if (ImGui::Button("Reload Scene")) {
		ChangeScene("Demo");
	}

	framework->GetCameraManager()->DrawImGui();
	ImGui::Separator();

	ImGui::Combo("Blend Mode", &currentBlendMode_, kBlendModeNames, static_cast<int>(BlendMode::kCountOfBlendMode));

	//パーティクル
	ImGui::Checkbox("Billboard", &particle_->GetUseBillboard());
	ImGui::DragFloat3("Emitter Translate", &particle_->GetEmitter().transform.translate.x, 0.1f);

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

	// 追従カメラの更新（追従対象の更新後、VPを使う処理の前に行う）
	CameraManager* cameraMgr = framework->GetCameraManager();
	cameraMgr->GetController()->Update(framework->GetDeltaTime());

	// パーティクルのWVP等を計算してResourceに書き込む
	particle_->Update(cameraMgr->GetViewProjection(), cameraMgr->GetRotateMatrix(),
		isParticleMoving_ ? framework->GetDeltaTime() : 0.0f, framework->GetDirectX()->GetFrameContext()->frameIndex);
}

void DemoScene::Draw(ID3D12GraphicsCommandList* commandList) {
	PipelineStateManager* psoMgr = GetFramework()->GetPipelineStateManager();
	BlendMode blendMode = static_cast<BlendMode>(currentBlendMode_);

	// 3D描画（パーティクル）
	psoMgr->SetPipeline(commandList, PipelineStateManager::kParticle, blendMode);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kDirectionalLight), light_->GetDirectionalLightAddress(*GetFramework()->GetDirectX()->GetFrameContext()->constantBuffers));
	particle_->Draw(commandList, *GetFramework()->GetDirectX()->GetFrameContext()->constantBuffers);

	// 2D描画
	psoMgr->SetPipeline(commandList, PipelineStateManager::kSprite, blendMode);
	commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kTexture), tex_.gpuHandle);
}

void DemoScene::Finalize() {
	// コントローラがこのシーンのメンバを指したまま残らないようにする
	CameraController* controller = GetFramework()->GetCameraManager()->GetController();
	controller->SetTarget(nullptr);
	controller->SetEnabled(false);

	light_.reset();
	particle_.reset();          // ID3D12Resource x1 (instancing)
	particleModelData_.reset(); // ID3D12Resource x3 (vertex, index, texture)
	sprite_.reset();            // 四角形メッシュは共有、定数バッファはアロケータなので専用のGPUリソースなし
	GetFramework()->GetAudio()->Stop(voice1_);
	voice1_ = Engine::kInvalidVoiceId;
	soundData1_.reset();
	tex_ = {};                  // ID3D12Resource (texture) と SRV スロット
	particleTex_ = {};
}

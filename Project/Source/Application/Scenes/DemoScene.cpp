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

	// === テクスチャ ===
	tex_ = assetFactory->LoadTexture("Resource/uvChecker.png");
	sphereTex_ = assetFactory->LoadTexture("Resource/monsterBall.png");

	// === 音声 ===
	AudioManager* audio = framework->GetAudio();
	soundData1_ = audio->LoadWave("Resource/Alarm01.wav");
	voice1_ = audio->Play(soundData1_);

	// === スプライト ===
	sprite_ = assetFactory->CreateSprite(640.0f, 360.0f);
	sprite_->SetTextureHandle(tex_.gpuHandle);

	// === 球（鏡面反射の確認用） ===
	sphereModelData_ = assetFactory->CreateSphereModelData();
	sphere_ = assetFactory->CreateModelInstance(sphereModelData_);
	sphere_->SetTextureHandle(sphereTex_.gpuHandle);
	sphere_->GetMaterial().SetEnableLighting(true);

	// === ライト ===
	light_ = std::make_unique<Light>();
}

void DemoScene::Update() {
	Framework* framework = GetFramework();

#ifdef USE_IMGUI
	ImGui::Begin("Debug");

	ImGui::Text("%f", ImGui::GetIO().Framerate);

	// シーンの作り直し
	if (ImGui::Button("Open Game Scene")) {
		ChangeScene("Game");
	}
	if (ImGui::Button("Reload Scene")) {
		ChangeScene("Demo");
	}

	framework->GetCameraManager()->DrawImGui();
	ImGui::Separator();

	ImGui::Combo("Blend Mode", &currentBlendMode_, kBlendModeNames, static_cast<int>(BlendMode::kCountOfBlendMode));

	//球
	ImGui::DragFloat3("Sphere Translate", &sphere_->GetTranslate().x, 0.1f);
	ImGui::DragFloat3("Sphere Rotate", &sphere_->GetRotate().x, 0.01f);
	ImGui::DragFloat3("Sphere Scale", &sphere_->GetScale().x, 0.01f);
	ImGui::ColorEdit4("Sphere Color", &sphere_->GetColor().x);
	ImGui::DragFloat("Shininess", &sphere_->GetMaterial().GetShininess(), 0.5f, 1.0f, 1000.0f);

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
}

void DemoScene::Draw(ID3D12GraphicsCommandList* commandList) {
	Framework* framework = GetFramework();
	PipelineStateManager* psoMgr = framework->GetPipelineStateManager();
	CameraManager* cameraMgr = framework->GetCameraManager();
	ConstantBufferAllocator& constantBuffers = *framework->GetDirectX()->GetFrameContext()->constantBuffers;
	BlendMode blendMode = static_cast<BlendMode>(currentBlendMode_);

	// 3D描画（球）: ライト(b1)とカメラ(b2)をセットしてから描く
	psoMgr->SetPipeline(commandList, PipelineStateManager::kObject3D, BlendMode::kNone);
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(PipelineStateManager::RootParameter::kDirectionalLight), light_->GetDirectionalLightAddress(constantBuffers));
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(PipelineStateManager::RootParameter::kCamera), cameraMgr->GetActiveCamera()->GetCameraAddress(constantBuffers));
	sphere_->Draw(cameraMgr->GetViewProjection());

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
	sphere_.reset();
	sphereModelData_.reset();   // 球メッシュの頂点・インデックスリソース
	sprite_.reset();            // 四角形メッシュは共有、定数バッファはアロケータなので専用のGPUリソースなし
	GetFramework()->GetAudio()->Stop(voice1_);
	voice1_ = Engine::kInvalidVoiceId;
	soundData1_.reset();
	tex_ = {};                  // ID3D12Resource (texture) と SRV スロット
	sphereTex_ = {};
}

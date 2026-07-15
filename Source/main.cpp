#include <Windows.h>
#include <d3d12.h>
#include<memory>

#include "WinApp.h"
#include "DirectXCommon.h"
#include "DebugUtil.h"
#include "D3DResourceLeakChecker.h"
#include "Light.h"
#include "TransformMatrix.h"
#include "ShaderManager.h"
#include "PipelineStateManager.h"
#include "TextureManager.h"
#include "Model.h"
#include "Sprite.h"
#include "ImGuiManager.h"
#include "CameraManager.h"
#include "AudioManager.h"
#include "Input.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif
#include "Matrix4x4.h"
#include "WorldTransform.h"

using namespace Engine;

/// <summary>
/// アプリケーションのエントリーポイント。各種システムの初期化・メインループ・終了処理を行う
/// </summary>
/// <param name="hInstance">アプリケーションのインスタンスハンドル</param>
/// <param name="hPrevInstance">未使用（常にnullptr）</param>
/// <param name="lpCmdLine">コマンドライン文字列</param>
/// <param name="nCmdShow">ウィンドウの表示状態</param>
/// <returns>アプリケーションの終了コード</returns>
// WinMain の定義にヘッダと整合する SAL 注釈を追加
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// === リソースリークチェッカー生成 ====================================================================

	D3DResourceLeakChecker leakCheck;

	// === COM初期化 ============================================================================

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	(void)hr;

	SetUnhandledExceptionFilter(DebugUtil::ExportDump);
	DebugUtil::CreateLogFile();

	// === WinApp初期化 ============================================================================

	WinApp* winApp = WinApp::GetInstance();
	winApp->Initialize();

	// === DirectX初期化 ============================================================================

	DirectXCommon* dx = DirectXCommon::GetInstance();
	dx->Initialize(winApp->GetHwnd(), WinApp::kClientWidth, WinApp::kClientHeight);

	// === Input初期化 =======================================================================

	Input* input = Input::GetInstance();
	input->Initialize(winApp->GetHInstance(), winApp->GetHwnd());

	// === Shader初期化 =============================================================================================

	auto shaderMgr = std::make_unique<ShaderManager>();
	shaderMgr->InitializeDXC();

	// === PSO初期化 =============================================================================================

	auto psoMgr = std::make_unique<PipelineStateManager>();
	ID3D12Device* device = dx->GetDevice();
	psoMgr->InitializeRootSignature(device);

	psoMgr->CreateObject3DPipeline(device, shaderMgr.get());
	psoMgr->CreateSpritePipeline(device, shaderMgr.get());

	// === ImGui初期化 ==========================================================

#ifdef USE_IMGUI
	ImGuiManager* imgui = ImGuiManager::GetInstance();
	imgui->Initialize(winApp->GetHwnd(), dx->GetDevice(), dx->GetSrvDescriptorHeap());
#endif

	// === TextureManager初期化 ==========================================================

	TextureManager* texMgr = TextureManager::GetInstance();

	// === テクスチャ ======================================================================================

	TextureHandle tex = texMgr->LoadTexture("resources/uvChecker.png");

	// === 音声データ =====================================================================================

	AudioManager* audioMgr = AudioManager::GetInstance();
	audioMgr->Initialize();

	// 音声読み込み
	SoundData soundData1 = audioMgr->LoadWave("Resources/Alarm01.wav");

	// 音声再生
	audioMgr->Play(soundData1);

	// === カメラ ==========================================================================================

	CameraManager* cameraMgr = CameraManager::GetInstance();
	cameraMgr->Initialize();

	// === オブジェクト ===========================================================================================

	// モデル
	auto axisModel = Model::CreateFromObj("resources", "axis.obj");
	dx->FlushCommands(); // コマンドリストをGPUに送信し完了を待つ

	// スプライト
	auto sprite = Sprite::Create(640.0f, 360.0f);
	sprite->SetTextureHandle(tex.gpuHandle);

	// === ライト ================================================================================

	auto light = std::make_unique<Light>();
	light->Initialize(device);

	// === UV Transform ====================================================================================

	WorldTransform uvTransformSprite{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	// === メインループ ====================================================================================

	MSG msg{};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

#ifdef USE_IMGUI
		imgui->BeginFrame();

		ImGui::Begin("Debug");

		cameraMgr->DrawImGui();
		ImGui::Separator();

		//モデル
		ImGui::ColorEdit4("Axis Color", &axisModel->GetColor().x);
		ImGui::DragFloat3("Axis Translate", &axisModel->GetTranslate().x, 0.1f);
		ImGui::DragFloat3("Axis Scale", &axisModel->GetScale().x, 0.1f);
		ImGui::DragFloat3("Axis Rotate", &axisModel->GetRotate().x, 0.1f);

		//ライト
		ImGui::ColorEdit4("Light Color", &light->GetDirectionalLight()->color.x);
		ImGui::DragFloat3("Light Direction", &light->GetDirectionalLight()->direction.x, 0.1f);
		ImGui::DragFloat("Light Intensity", &light->GetDirectionalLight()->intensity, 0.1f);

		ImGui::End();
#endif

		// ====================================================================================================
		// 更新処理
		// ====================================================================================================

		// スプライトUV更新
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		sprite->GetMaterial().GetUVTransform() = uvTransformMatrix;

		// ====================================================================================================
		// 描画処理
		// ====================================================================================================

		dx->BeginFrame();

		// キー状態更新
		input->Update();

		// カメラ更新（F1切り替え含む）
		cameraMgr->Update();

		// DescriptorHeap・RootSignature・PSOのセット
		auto* commandList = dx->GetCommandList();
		ID3D12DescriptorHeap* heaps[] = { dx->GetSrvDescriptorHeap() };
		commandList->SetDescriptorHeaps(1, heaps);

		// 3D描画
		psoMgr->SetPipeline(commandList, PipelineStateManager::kObject3D);
		commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(PipelineStateManager::RootParameter::kDirectionalLight), light->GetDirectionalLightAddress());
		axisModel->Draw(cameraMgr->GetViewProjection());

		// 2D描画
		psoMgr->SetPipeline(commandList, PipelineStateManager::kSprite);
		commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(PipelineStateManager::RootParameter::kTexture), tex.gpuHandle);

#ifdef USE_IMGUI
		imgui->EndFrame(dx->GetCommandList());
#endif

		dx->EndFrame();
	}

#ifdef USE_IMGUI
	imgui->Finalize();
#endif

	// D3D12リソースをデバイス解放より前に明示的に解放する
	// これらが生きていると device の参照カウントが残り LIVE_DEVICE 警告でクラッシュする
	light.reset();        // ~Light() で Unmap + Release
	sprite.reset();       // ID3D12Resource x3 (vertex, material, wvp)
	axisModel.reset();  // ID3D12Resource x3 (vertex, material, wvp)
	audioMgr->Unload(&soundData1);
	audioMgr->Finalize();
	input->Finalize();
	tex = {};             // ID3D12Resource x2 (texture, intermediate)
	psoMgr.reset();       // ID3D12PipelineState x2 + ID3D12RootSignature x1

	dx->Finalize();
	CoUninitialize();

	return 0;
}
#include <Windows.h>
#include <d3d12.h>
#include<memory>

#include "WinApp.h"
#include "DirectXCommon.h"
#include "DebugUtil.h"
#include "D3D12_Util.h"
#include "Light.h"
#include "ShaderManager.h"
#include "PipelineStateManager.h"
#include "TextureManager.h"
#include "Model.h"
#include "Sprite.h"
#include "ImGuiManager.h"
#include"Camera.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// COM 初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	(void)hr;

	SetUnhandledExceptionFilter(DebugUtil::ExportDump);
	DebugUtil::CreateLogFile();

	// WinApp & DirectX 初期化
	WinApp* winApp = WinApp::GetInstance();
	winApp->Initialize();

	DirectXCommon* dx = DirectXCommon::GetInstance();
	dx->Initialize(winApp->GetHwnd(), WinApp::kClientWidth, WinApp::kClientHeight);

	// Shader / PSO 管理
	auto shaderMgr = std::make_unique<ShaderManager>();
	shaderMgr->InitializeDXC();

	auto psoMgr = std::make_unique<PipelineStateManager>();
	ID3D12Device* device = dx->GetDevice();
	psoMgr->InitializeRootSignature(device);

	// === 3D 用 PSO =======================================================================================

	IDxcBlob* vs = shaderMgr->Compile(L"Object3d.VS.hlsl", L"vs_6_0");
	IDxcBlob* ps = shaderMgr->Compile(L"Object3d.PS.hlsl", L"ps_6_0");

	D3D12_INPUT_ELEMENT_DESC inputElements[3] = {};
	inputElements[0].SemanticName = "POSITION";
	inputElements[0].SemanticIndex = 0;
	inputElements[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElements[1].SemanticName = "TEXCOORD";
	inputElements[1].SemanticIndex = 0;
	inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElements[2].SemanticName = "NORMAL";
	inputElements[2].SemanticIndex = 0;
	inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.pInputElementDescs = inputElements;
	inputLayout.NumElements = _countof(inputElements);

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterDesc{};
	rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	psoMgr->CreateGraphicsPipeline(
		"Object3D",
		device,
		vs,
		ps,
		inputLayout,
		blendDesc,
		rasterDesc,
		depthDesc,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_D24_UNORM_S8_UINT
	);

	// === 2D 用 PSO =======================================================================================

	D3D12_DEPTH_STENCIL_DESC depthNone{};
	depthNone.DepthEnable = false;
	depthNone.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthNone.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	psoMgr->CreateGraphicsPipeline(
		"Sprite",
		device,
		vs,
		ps,
		inputLayout,
		blendDesc,
		rasterDesc,
		depthNone,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_UNKNOWN
	);

	// === ImGui初期化 ==========================================================

#ifdef USE_IMGUI
	ImGuiManager* imgui = ImGuiManager::GetInstance();
	imgui->Initialize(winApp->GetHwnd(), dx->GetDevice(), dx->GetSrvDescriptorHeap());
#endif

	// === TextureManager初期化 ==========================================================

	TextureManager* texMgr = TextureManager::GetInstance();

	// === カメラ ==========================================================================================

	std::unique_ptr<Camera> camera = std::make_unique<Camera>();
	camera->Update();

	// === テクスチャ ======================================================================================

	TextureHandle tex = texMgr->LoadTexture("resources/uvChecker.png");
	TextureHandle tex2 = texMgr->LoadTexture("resources/monsterBall.png");
	dx->FlushCommands();

	// === モデル / スプライト / オブジェクト =============================================================

	auto sphereModel = std::unique_ptr<Model>(Model::CreateSphere());
	sphereModel->SetTextureHandle(tex2.gpuHandle);

	auto sprite = std::unique_ptr<Sprite>(Sprite::Create(640.0f, 360.0f));
	sprite->SetTextureHandle(tex.gpuHandle);

	// === DirectionalLight ================================================================================

	auto light = std::make_unique<Light>();
	light->Initialize(device);

	// === メインループ ====================================================================================

	bool useMonsterBall = true;

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

		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		//球
		ImGui::ColorEdit4("Sphere Color", &sphereModel->GetColor().x);
		ImGui::DragFloat3("Sphere Translate", &sphereModel->GetTranslate().x, 0.1f);
		ImGui::DragFloat3("Sphere Scale", &sphereModel->GetScale().x, 0.1f);
		ImGui::DragFloat3("Sphere Rotate", &sphereModel->GetRotate().x, 0.1f);

		//スプライト
		ImGui::ColorEdit4("Sprite Color", &sprite->GetColor().x);
		ImGui::DragFloat2("Sprite Translate", &sprite->GetTranslate().x, 1.0f);

		//ライト
		ImGui::ColorEdit4("Light Color", &light->GetDirectionalLight()->color.x);
		ImGui::DragFloat3("Light Direction", &light->GetDirectionalLight()->direction.x, 0.1f);
		ImGui::DragFloat("Light Intensity", &light->GetDirectionalLight()->intensity, 0.1f);

		//カメラ
		ImGui::DragFloat3("Camera Translate", &camera->GetTranslate().x, 0.1f);
		ImGui::DragFloat3("Camera Rotate", &camera->GetRotate().x, 0.1f);

		ImGui::End();
#endif

		// ====================================================================================================
		// 更新処理
		// ====================================================================================================

		//球を回転させる
		sphereModel->Update();

		// カメラ更新
		camera->Update();

		// ====================================================================================================
		// 描画処理
		// ====================================================================================================

		dx->BeginFrame();

		// DescriptorHeap・RootSignature・PSOのセット
		auto* commandList = dx->GetCommandList();
		ID3D12DescriptorHeap* heaps[] = { dx->GetSrvDescriptorHeap() };
		commandList->SetDescriptorHeaps(1, heaps);

		// 3D描画
		psoMgr->SetPipeline(commandList, "Object3D");
		commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? tex2.gpuHandle : tex.gpuHandle);
		commandList->SetGraphicsRootConstantBufferView(3, light->GetDirectionalLightAddress());
		sphereModel->Draw(camera->GetViewProjection());

		// 2D描画
		psoMgr->SetPipeline(commandList, "Sprite");
		commandList->SetGraphicsRootDescriptorTable(2,tex.gpuHandle);
		sprite->Draw(camera->GetSpriteViewProjection());

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
	sphereModel.reset();  // ID3D12Resource x3 (vertex, material, wvp)
	tex = {};             // ID3D12Resource x2 (texture, intermediate)
	tex2 = {};            // ID3D12Resource x2 (texture, intermediate)
	psoMgr.reset();       // ID3D12PipelineState x2 + ID3D12RootSignature x1

	dx->Finalize();
	CoUninitialize();

	return 0;
}
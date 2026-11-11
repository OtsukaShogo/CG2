#include "Framework.h"

#include "WinApp.h"
#include "DebugUtil.h"
#include "ImGuiManager.h"
#include "../Input/Input.h"
#include "../Audio/AudioManager.h"
#include "../3D/CameraManager.h"

namespace Engine {

Framework::Framework() = default;
Framework::~Framework() = default;

void Framework::Run() {
	Initialize();

	while (!endRequest_) {
		Update();
		// Update中にRequestEnd()された場合は描画せずに抜ける
		if (endRequest_) {
			break;
		}
		Draw();
	}

	Finalize();
}

void Framework::Initialize() {
	// === WinApp ===
	winApp_ = WinApp::GetInstance();
	winApp_->Initialize();

	// === DirectX ===
	dx_ = std::make_unique<DirectXCommon>();
	dx_->Initialize(winApp_->GetHwnd(), WinApp::kClientWidth, WinApp::kClientHeight);

	assetFactory_.Initialize(dx_.get());

	// === Input ===
	input_ = Input::GetInstance();
	input_->Initialize(winApp_);

	// === Shader / PSO ===
	shaderMgr_ = std::make_unique<ShaderManager>();
	shaderMgr_->InitializeDXC();

	psoMgr_ = std::make_unique<PipelineStateManager>();
	ID3D12Device* device = dx_->GetDevice();
	psoMgr_->InitializeRootSignature(device);
	psoMgr_->CreateObject3DPipeline(device, shaderMgr_.get());
	psoMgr_->CreateSpritePipeline(device, shaderMgr_.get());
	psoMgr_->CreateParticlePipeline(device, shaderMgr_.get());

	// === ImGui ===
#ifdef USE_IMGUI
	imgui_ = ImGuiManager::GetInstance();
	imgui_->Initialize(winApp_->GetHwnd(), dx_->GetDevice(), dx_->GetSrvDescriptorHeap());
#endif

	// === Audio ===
	audio_ = AudioManager::GetInstance();
	audio_->Initialize();

	// === Camera ===
	cameraMgr_ = CameraManager::GetInstance();
	cameraMgr_->Initialize();

	// === Asset ===
	assetManager_.Initialize(&assetFactory_);

	sceneMgr_.Initialize(this);

	initialized_ = true;

	// === ゲーム側の初期化（シーン登録・最初のシーン指定） ===
	OnInitialize();

	// 最初のシーンを生成する（コマンドリストは初期化直後で開いているのでそのままアップロードできる）
	sceneMgr_.ApplyPendingChange();

	// 初期化中に積まれたアップロード等のコマンドをGPUに送信し完了を待つ
	dx_->FlushCommands();

	// 最初のフレームの dt が初期化時間を含まないようにする
	lastFrameTime_ = std::chrono::steady_clock::now();
}

void Framework::UpdateDeltaTime() {
	auto now = std::chrono::steady_clock::now();
	float dt = std::chrono::duration<float>(now - lastFrameTime_).count();
	lastFrameTime_ = now;

	deltaTime_ = dt > kMaxDeltaTime ? kMaxDeltaTime : dt;
}

void Framework::Update() {
	// Windowsのメッセージ処理（終了メッセージを受け取ったらループを抜ける）
	if (winApp_->ProcessMessage()) {
		RequestEnd();
		return;
	}

	UpdateDeltaTime();

	// シーン切り替え要求があれば、描画開始前に行う
	// （前フレームのGPU完了後なので、旧シーンのリソースを安全に解放でき、新シーンの読み込みもここでGPUへ反映する）
	if (sceneMgr_.HasPendingChange()) {
		dx_->BeginUpload();
		sceneMgr_.ApplyPendingChange();
		dx_->FlushCommands();

		// 読み込み時間が次フレームのdtに混ざらないようにする
		lastFrameTime_ = std::chrono::steady_clock::now();
	}

#ifdef USE_IMGUI
	imgui_->BeginFrame();
#endif

	input_->Update();

	// カメラ更新（F1でデバッグカメラ切り替え）。ゲーム側のOnUpdateより先に行う
	cameraMgr_->Update();

	OnUpdate();
	sceneMgr_.Update();
}

void Framework::Draw() {
	dx_->BeginFrame();

	// DescriptorHeapのセット
	auto* commandList = dx_->GetCommandList();
	ID3D12DescriptorHeap* heaps[] = { dx_->GetSrvDescriptorHeap() };
	commandList->SetDescriptorHeaps(1, heaps);

	OnDraw(commandList);
	sceneMgr_.Draw(commandList);

#ifdef USE_IMGUI
	imgui_->EndFrame(commandList);
#endif

	dx_->EndFrame();
}

void Framework::Finalize() {
	if (!initialized_) {
		return;
	}

#ifdef USE_IMGUI
	imgui_->Finalize();
#endif

	// D3D12リソースをデバイス解放より前に明示的に解放する
	// これらが生きていると device の参照カウントが残り LIVE_DEVICE 警告でクラッシュする
	sceneMgr_.Finalize();
	OnFinalize();

	assetManager_.Clear(); // ModelData（メッシュ・テクスチャ）を解放
	audio_->Finalize();
	input_->Finalize();
	psoMgr_.reset();       // ID3D12PipelineState + ID3D12RootSignature

	dx_->Finalize();
	winApp_->Finalize();

	initialized_ = false;
}

} // namespace Engine

#include "Framework.h"

#include "WinApp.h"
#include "DebugUtil.h"
#include "ImGuiManager.h"
#include "../Input/Input.h"
#include "../Audio/AudioManager.h"
#include "../3D/CameraManager.h"
#include "../2D/Sprite.h"

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

	TextureManager::GetInstance()->Initialize(DirectXCommon::kSrvHeapSize);
	assetFactory_.Initialize(dx_.get());

	// デフォルトテクスチャ（白）を作成する。初期化直後でコマンドリストが開いているのでそのままアップロードできる
	TextureManager::GetInstance()->CreateDefaultTextures(
		dx_->GetDevice(), dx_->GetFrameContext()->commandList, dx_->GetSrvDescriptorHeap(), dx_->GetDescriptorSizeSRV());

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
	TextureManager::GetInstance()->ReleaseUploadBuffers(); // 転送完了後なのでアップロード用バッファを解放できる

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

	// 今フレームが使うスロットのGPU処理（2フレーム前）が終わるまで待つ。以降、CPUが書くリソースを安全に更新できる
	dx_->WaitForFrameSlot();

	// シーン切り替え要求があれば、描画開始前に行う
	// （GPUの完了を待ってから旧シーンのリソースを解放し、新シーンの読み込みもここでGPUへ反映する）
	if (sceneMgr_.HasPendingChange()) {
		dx_->WaitIdle(); // 旧シーンのリソースをGPUが使用中でないことを保証する
		dx_->BeginUpload();
		sceneMgr_.ApplyPendingChange();
		dx_->FlushCommands();
		TextureManager::GetInstance()->ReleaseUploadBuffers();

		// 読み込み時間が次フレームのdtに混ざらないようにする
		lastFrameTime_ = std::chrono::steady_clock::now();
	}

#ifdef USE_IMGUI
	imgui_->BeginFrame();
	// ImGuiが入力を使っている間（入力欄への文字入力・ウィンドウ操作など）は、ゲームへ入力を渡さない
	input_->SetKeyboardBlocked(imgui_->WantCaptureKeyboard());
	input_->SetMouseBlocked(imgui_->WantCaptureMouse());
#endif

	input_->Update();
	audio_->Update(); // 再生が終わった音声の後始末

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

	// 以降で解放するリソースを、GPUが使用中でないようにする（描画を2フレーム先行させているため）
	dx_->WaitIdle();

#ifdef USE_IMGUI
	imgui_->Finalize();
#endif

	// D3D12リソースをデバイス解放より前に明示的に解放する
	// これらが生きていると device の参照カウントが残り LIVE_DEVICE 警告でクラッシュする
	sceneMgr_.Finalize();
	OnFinalize();

	Sprite::ReleaseSharedResources(); // スプライト共有の四角形メッシュ
	assetManager_.Clear(); // ModelData（メッシュ・テクスチャ）を解放
	TextureManager::GetInstance()->Finalize(); // デフォルトテクスチャ・キャッシュを解放
	audio_->Finalize();
	input_->Finalize();
	psoMgr_.reset();       // ID3D12PipelineState + ID3D12RootSignature

	dx_->Finalize();
	winApp_->Finalize();

	initialized_ = false;
}

} // namespace Engine

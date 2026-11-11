#pragma once
#include <d3d12.h>
#include <memory>
#include <chrono>

#include "DirectXCommon.h"
#include "ShaderManager.h"
#include "PipelineStateManager.h"
#include "../Graphics/AssetFactory.h"
#include "../Graphics/AssetManager.h"
#include "../Scene/SceneManager.h"

namespace Engine {

class WinApp;
class Input;
class AudioManager;
class CameraManager;
class ImGuiManager;

/// <summary>
/// エンジン全体の初期化・メインループ・終了処理を担う基底クラス。
/// ゲーム側はこのクラスを継承し、OnInitialize / OnUpdate / OnDraw / OnFinalize を実装して Run() を呼ぶ
/// </summary>
class Framework {
public:
	Framework();
	virtual ~Framework();

	//コピー禁止
	Framework(const Framework&) = delete;
	Framework& operator=(const Framework&) = delete;

	/// <summary>
	/// 初期化→メインループ→終了処理を一括で実行する（終了するまで戻らない）
	/// </summary>
	void Run();

	/// <summary>
	/// ゲームループの終了を要求する（次のフレーム頭でループを抜ける）
	/// </summary>
	void RequestEnd() { endRequest_ = true; }

	/// <summary>
	/// 直前フレームの経過時間（秒）を取得する。極端に大きい値は上限でクランプされる
	/// </summary>
	[[nodiscard]] float GetDeltaTime() const { return deltaTime_; }

protected:
	/// <summary>
	/// エンジン初期化後に1度だけ呼ばれる。シーンの登録や最初のシーンの指定、共通アセットの読み込みを行う
	/// （ここで行ったテクスチャ読み込み等は呼び出し後に自動でGPUへ反映される）
	/// </summary>
	virtual void OnInitialize() = 0;

	/// <summary>
	/// 毎フレーム、シーンの更新より先に呼ばれる共通の更新処理。入力・カメラは更新済みで、ImGuiのフレームも開始済み
	/// </summary>
	virtual void OnUpdate() {}

	/// <summary>
	/// 毎フレーム、シーンの描画より先に呼ばれる共通の描画処理。SRVヒープはセット済みで、ImGuiの描画はシーンの描画後に行われる
	/// </summary>
	/// <param name="commandList">今フレームのコマンドリスト</param>
	virtual void OnDraw(ID3D12GraphicsCommandList* commandList) { (void)commandList; }

	/// <summary>
	/// 終了時に1度だけ呼ばれる（シーンの終了後、デバイス解放より前）。
	/// D3D12リソースを保持している場合はここで解放する
	/// </summary>
	virtual void OnFinalize() {}

public:
	//サブシステムへのアクセサ（シーンからも利用する）
	[[nodiscard]] DirectXCommon* GetDirectX() const { return dx_.get(); }
	[[nodiscard]] PipelineStateManager* GetPipelineStateManager() const { return psoMgr_.get(); }
	[[nodiscard]] AssetFactory* GetAssetFactory() { return &assetFactory_; }
	[[nodiscard]] AssetManager* GetAssetManager() { return &assetManager_; }
	[[nodiscard]] WinApp* GetWinApp() const { return winApp_; }
	[[nodiscard]] Input* GetInput() const { return input_; }
	[[nodiscard]] AudioManager* GetAudio() const { return audio_; }
	[[nodiscard]] CameraManager* GetCameraManager() const { return cameraMgr_; }
	[[nodiscard]] SceneManager* GetSceneManager() { return &sceneMgr_; }

private:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	/// <summary>
	/// 前回呼び出しからの経過時間を計測して deltaTime_ に反映する
	/// </summary>
	void UpdateDeltaTime();

private:
	// 1フレームの経過時間の上限（デバッグ停止やウィンドウ移動後の大ジャンプ防止）
	static constexpr float kMaxDeltaTime = 0.1f;

	WinApp* winApp_ = nullptr;
	Input* input_ = nullptr;
	AudioManager* audio_ = nullptr;
	CameraManager* cameraMgr_ = nullptr;
#ifdef USE_IMGUI
	ImGuiManager* imgui_ = nullptr;
#endif

	std::unique_ptr<DirectXCommon> dx_;
	std::unique_ptr<ShaderManager> shaderMgr_;
	std::unique_ptr<PipelineStateManager> psoMgr_;
	AssetFactory assetFactory_;
	AssetManager assetManager_;
	SceneManager sceneMgr_;

	std::chrono::steady_clock::time_point lastFrameTime_;
	float deltaTime_ = 1.0f / 60.0f;
	bool endRequest_ = false;
	bool initialized_ = false;
};

} // namespace Engine

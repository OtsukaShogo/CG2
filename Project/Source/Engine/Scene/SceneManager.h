#pragma once
#include <d3d12.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Scene.h"

namespace Engine {

class Framework;

/// <summary>
/// シーンの登録・切り替え・更新・描画を管理するクラス。
/// 切り替えは要求した次のフレームの頭で行われるため、Update中に ChangeScene() を呼んでも安全
/// </summary>
class SceneManager {
public:
	using SceneFactory = std::function<std::unique_ptr<Scene>()>;

	SceneManager() = default;
	~SceneManager();

	//コピー禁止
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	/// <summary>
	/// 初期化する
	/// </summary>
	/// <param name="framework">シーンへ渡すFramework</param>
	void Initialize(Framework* framework);

	/// <summary>
	/// シーンを名前付きで登録する
	/// </summary>
	/// <param name="name">シーン名</param>
	/// <param name="factory">シーンを生成する関数</param>
	void Register(const std::string& name, SceneFactory factory);

	/// <summary>
	/// シーン型Tを名前付きで登録する（Tはデフォルトコンストラクタで生成される）
	/// </summary>
	template<class T>
	void Register(const std::string& name) {
		Register(name, []() { return std::make_unique<T>(); });
	}

	/// <summary>
	/// 指定した名前のシーンへの切り替えを要求する（実際の切り替えは ApplyPendingChange() で行われる）
	/// 現在と同じ名前を指定すると、シーンの作り直し（リロード）になる
	/// </summary>
	/// <param name="name">登録済みのシーン名</param>
	void ChangeScene(const std::string& name);

	/// <summary>
	/// 切り替え要求があるかを取得する
	/// </summary>
	[[nodiscard]] bool HasPendingChange() const { return !nextSceneName_.empty(); }

	/// <summary>
	/// 切り替え要求を実行する（現シーンのFinalize → 新シーンの生成とInitialize）
	/// </summary>
	void ApplyPendingChange();

	/// <summary>
	/// 現在のシーンを更新する
	/// </summary>
	void Update();

	/// <summary>
	/// 現在のシーンを描画する
	/// </summary>
	/// <param name="commandList">今フレームのコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 現在のシーンを終了して破棄する
	/// </summary>
	void Finalize();

	/// <summary>
	/// 現在のシーン名を取得する（シーンがない場合は空文字）
	/// </summary>
	[[nodiscard]] const std::string& GetCurrentSceneName() const { return currentSceneName_; }

private:
	Framework* framework_ = nullptr;
	std::unordered_map<std::string, SceneFactory> factories_;
	std::unique_ptr<Scene> currentScene_;
	std::string currentSceneName_;
	std::string nextSceneName_;
};

} // namespace Engine

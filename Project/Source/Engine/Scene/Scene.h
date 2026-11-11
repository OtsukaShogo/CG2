#pragma once
#include <d3d12.h>
#include <string>

namespace Engine {

class Framework;
class SceneManager;

/// <summary>
/// シーン（タイトル・ゲーム・リザルトなど1つの画面単位）の基底クラス。
/// 継承して Update / Draw を実装し、SceneManager に登録して使う
/// </summary>
class Scene {
public:
	virtual ~Scene() = default;

	/// <summary>
	/// シーン開始時に1度だけ呼ばれる。アセット読み込みやオブジェクト生成を行う
	/// （ここで行ったテクスチャ読み込み等は呼び出し後に自動でGPUへ反映される）
	/// </summary>
	virtual void Initialize() {}

	/// <summary>
	/// 毎フレーム呼ばれる更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 毎フレーム呼ばれる描画処理
	/// </summary>
	/// <param name="commandList">今フレームのコマンドリスト</param>
	virtual void Draw(ID3D12GraphicsCommandList* commandList) = 0;

	/// <summary>
	/// シーン終了時に1度だけ呼ばれる。D3D12リソースを保持している場合はここで解放する
	/// </summary>
	virtual void Finalize() {}

protected:
	/// <summary>
	/// エンジンのサブシステムへアクセスするための Framework を取得する
	/// </summary>
	[[nodiscard]] Framework* GetFramework() const { return framework_; }

	/// <summary>
	/// 指定した名前のシーンへ切り替える。切り替えは次フレームの頭で行われる
	/// </summary>
	/// <param name="name">SceneManager に登録済みのシーン名</param>
	void ChangeScene(const std::string& name);

private:
	friend class SceneManager;

	Framework* framework_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
};

} // namespace Engine

#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include "VertexData.h"

namespace Engine {

class Material;

/// <summary>
/// 頂点・インデックスデータとマテリアルを保持し、GPUへのアップロードと描画を行うクラス
/// </summary>
class Mesh {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Mesh();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Mesh();

	/// <summary>
	/// 球メッシュを生成する
	/// </summary>
	/// <param name="subdivision">球の分割数</param>
	void CreateSphere(uint32_t subdivision = 16);

	/// <summary>
	/// 矩形メッシュを生成する
	/// </summary>
	/// <param name="width">矩形の幅</param>
	/// <param name="height">矩形の高さ</param>
	void CreateRect(float width, float height);

	/// <summary>
	/// objファイルを読み込み、頂点・マテリアルデータを生成する
	/// </summary>
	/// <param name="directoryPath">objファイルが存在するディレクトリパス</param>
	/// <param name="filename">読み込むobjファイル名</param>
	void LoadObjFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// 頂点・インデックスデータをGPUへ転送する
	/// </summary>
	void Upload();

	/// <summary>
	/// メッシュを描画する
	/// </summary>
	/// <param name="commandList">描画コマンドを積むコマンドリスト</param>
	void Draw(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// 頂点数を取得する
	/// </summary>
	/// <returns>頂点数</returns>
	uint32_t GetVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }

	/// <summary>
	/// マテリアルが参照するテクスチャファイルパスを取得する
	/// </summary>
	/// <returns>テクスチャファイルパス</returns>
	const std::string& GetTextureFilePath() const;

private:
	std::vector<VertexData> vertices_;
	std::unique_ptr<Material> material_;

	std::vector<uint32_t>   indices_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW ibv_{};
};

} // namespace Engine

#pragma once
#include <vector>
#include <memory>
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <sstream>
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
	/// <param name="device">D3D12デバイス</param>
	void Upload(ID3D12Device* device);

	/// <summary>
	/// メッシュを描画する
	/// </summary>
	/// <param name="commandList">描画コマンドを積むコマンドリスト</param>
	/// <param name="instanceCount">インスタンシング描画するインスタンス数</param>
	void Draw(ID3D12GraphicsCommandList* commandList, uint32_t instanceCount = 1) const;

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
	/// <summary>
	/// "v"行（頂点座標）を解析し、positionsへ追加する
	/// </summary>
	static void ParseVertexLine(std::istringstream& s, std::vector<Vector4>& positions);

	/// <summary>
	/// "vt"行（UV座標）を解析し、texcoordsへ追加する
	/// </summary>
	static void ParseTexcoordLine(std::istringstream& s, std::vector<Vector2>& texcoords);

	/// <summary>
	/// "vn"行（法線）を解析し、normalsへ追加する
	/// </summary>
	static void ParseNormalLine(std::istringstream& s, std::vector<Vector3>& normals);

	/// <summary>
	/// "f"行（面）を解析し、対応する頂点をvertices_へ追加する
	/// </summary>
	void ParseFaceLine(
		std::istringstream& s,
		const std::vector<Vector4>& positions,
		const std::vector<Vector2>& texcoords,
		const std::vector<Vector3>& normals);

	/// <summary>
	/// "mtllib"行（マテリアルテンプレートライブラリ）を解析し、material_を読み込む
	/// </summary>
	void ParseMtllibLine(std::istringstream& s, const std::string& directoryPath);

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

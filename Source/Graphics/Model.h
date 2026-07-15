#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include "TransformMatrix.h"
#include "WorldTransform.h"
#include "Mesh.h"
#include "Material.h"

namespace Engine {

/// <summary>
/// メッシュ・マテリアル・ワールドトランスフォームを持つ3Dモデルを表すクラス
/// </summary>
class Model {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Model();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Model();

	/// <summary>
	/// 球メッシュを持つモデルを生成する
	/// </summary>
	/// <returns>生成したモデル</returns>
	[[nodiscard]] static std::unique_ptr<Model> CreateSphere();

	/// <summary>
	/// objファイルを読み込んでモデルを生成する
	/// </summary>
	/// <param name="directoryPath">objファイルが存在するディレクトリパス</param>
	/// <param name="filename">読み込むobjファイル名</param>
	/// <returns>生成したモデル</returns>
	[[nodiscard]] static std::unique_ptr<Model> CreateFromObj(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// ワールド行列・WVP行列を更新する
	/// </summary>
	void Update();

	/// <summary>
	/// モデルを描画する
	/// </summary>
	/// <param name="viewProjection">適用するビュープロジェクション行列</param>
	void Draw(const Matrix4x4& viewProjection);

public:

	/// <summary>
	/// マテリアルの色を取得する（変更可能）
	/// </summary>
	/// <returns>マテリアルの色</returns>
	Vector4& GetColor() { return material_.GetColor(); }

	/// <summary>
	/// 座標を取得する（変更可能）
	/// </summary>
	/// <returns>座標</returns>
	Vector3& GetTranslate() { return worldTransform_.translate; }

	/// <summary>
	/// 拡大縮小率を取得する（変更可能）
	/// </summary>
	/// <returns>拡大縮小率</returns>
	Vector3& GetScale() { return worldTransform_.scale; }

	/// <summary>
	/// 回転角を取得する（変更可能）
	/// </summary>
	/// <returns>回転角</returns>
	Vector3& GetRotate() { return worldTransform_.rotate; }

	/// <summary>
	/// マテリアルの色を設定する
	/// </summary>
	/// <param name="color">設定する色</param>
	void SetColor(const Vector4& color) { material_.SetColor(color); }

	/// <summary>
	/// テクスチャのGPUディスクリプタハンドルを設定する
	/// </summary>
	/// <param name="handle">設定するGPUディスクリプタハンドル</param>
	void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { material_.SetTextureHandle(handle); }

private:

	/// <summary>
	/// WVP行列用の定数バッファリソースを生成する
	/// </summary>
	void CreateWvpBuffer();

private:
	Mesh mesh_;
	Material material_;
	WorldTransform worldTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	TransformationMatrix* wvpData_ = nullptr;
};

} // namespace Engine

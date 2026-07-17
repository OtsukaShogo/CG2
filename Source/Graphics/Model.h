#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include "TransformMatrix.h"
#include "WorldTransform.h"
#include "Material.h"

namespace Engine {

class ModelData;

/// <summary>
/// ModelDataが持つ描画データを参照し、マテリアル・ワールドトランスフォームなど
/// インスタンス固有のデータを保持する3Dモデルのインスタンス
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
	/// ModelDataを参照するモデルインスタンスを生成する
	/// </summary>
	/// <param name="modelData">参照する共有描画データ（shared_ptrで保持するため、AssetManagerより先に破棄されても問題ない）</param>
	/// <param name="device">D3D12デバイス</param>
	/// <returns>生成したモデル</returns>
	[[nodiscard]] static std::unique_ptr<Model> CreateInstance(std::shared_ptr<const ModelData> modelData, ID3D12Device* device);

	/// <summary>
	/// ワールド行列・WVP行列を更新する
	/// </summary>
	void Update();

	/// <summary>
	/// モデルを描画する
	/// </summary>
	/// <param name="viewProjection">適用するビュープロジェクション行列</param>
	/// <param name="commandList">描画コマンドを積むコマンドリスト</param>
	void Draw(const Matrix4x4& viewProjection, ID3D12GraphicsCommandList* commandList);

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
	/// <param name="device">D3D12デバイス</param>
	void CreateWvpBuffer(ID3D12Device* device);

private:
	std::shared_ptr<const ModelData> modelData_; // 参照カウント方式。AssetManagerが破棄されてもModelが生きている限りデータは解放されない
	Material material_;
	WorldTransform worldTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	TransformationMatrix* wvpData_ = nullptr;
};

} // namespace Engine

#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include "Vector4.h"
#include "TransformMatrix.h"
#include "WorldTransform.h"
#include "Mesh.h"
#include "Material.h"

namespace Engine {

/// <summary>
/// 2D描画用のスプライトを表すクラス
/// </summary>
class Sprite {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Sprite();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Sprite();

	/// <summary>
	/// 指定サイズの矩形スプライトを生成する
	/// </summary>
	/// <param name="width">スプライトの幅</param>
	/// <param name="height">スプライトの高さ</param>
	/// <returns>生成したスプライト</returns>
	[[nodiscard]] static std::unique_ptr<Sprite> Create(float width, float height);

	/// <summary>
	/// スプライトを描画する
	/// </summary>
	/// <param name="viewProjection">適用するビュープロジェクション行列</param>
	void Draw(const Matrix4x4& viewProjection);

public:

	/// <summary>
	/// マテリアルの色を取得する（変更可能）
	/// </summary>
	/// <returns>マテリアルの色</returns>
	Vector4&  GetColor()    { return material_.GetColor(); }

	/// <summary>
	/// 座標を取得する（変更可能）
	/// </summary>
	/// <returns>座標</returns>
	Vector3&  GetTranslate(){ return worldTransform_.translate; }

	/// <summary>
	/// マテリアルを取得する
	/// </summary>
	/// <returns>マテリアル</returns>
	Material& GetMaterial() { return material_; }

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

	Mesh     mesh_;
	Material material_;
	WorldTransform worldTransform_ = { { 1.0f, 1.0f, 1.0f } ,{ 0.0f, 0.0f, 0.0f } ,{ 0.0f, 0.0f, 0.0f } };

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	TransformationMatrix* wvpData_ = nullptr;
};

} // namespace Engine

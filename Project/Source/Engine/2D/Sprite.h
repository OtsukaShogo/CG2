#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include "../Math/Vector2.h"
#include "../Math/Vector4.h"
#include "../Math/TransformMatrix.h"
#include "../Math/WorldTransform.h"
#include "../Graphics/Mesh.h"
#include "../Graphics/Material.h"
#include "../Core/FrameContext.h"

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
	/// <param name="device">D3D12デバイス</param>
	/// <param name="frameContext">「今フレームで使うべきコマンドリスト」を保持する共有オブジェクト</param>
	/// <returns>生成したスプライト</returns>
	[[nodiscard]] static std::unique_ptr<Sprite> Create(
		float width, float height,
		ID3D12Device* device,
		std::shared_ptr<const FrameContext> frameContext);

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

	/// <summary>
	/// スプライトの大きさ（ピクセル）を取得する
	/// </summary>
	[[nodiscard]] Vector2 GetSize() const { return size_; }

	/// <summary>
	/// スプライトの大きさ（ピクセル）を設定する。四角形のメッシュは共有なので、描画時に拡大して反映される
	/// </summary>
	void SetSize(const Vector2& size) { size_ = size; }

private:

	std::shared_ptr<const Mesh> mesh_; // 1x1の四角形。全スプライトで共有する
	Material material_;
	Vector2  size_ = { 1.0f, 1.0f };
	std::shared_ptr<const FrameContext> frameContext_; // Draw()時に「今フレームのcommandList」を都度参照するための共有オブジェクト
	WorldTransform worldTransform_ = { { 1.0f, 1.0f, 1.0f } ,{ 0.0f, 0.0f, 0.0f } ,{ 0.0f, 0.0f, 0.0f } };
};

} // namespace Engine

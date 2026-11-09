#pragma once
#include <d3d12.h>
#include <memory>
#include <string>

#include "Mesh.h"
#include "TextureManager.h"

namespace Engine {

/// <summary>
/// 複数のModelインスタンス間で共有される、パス単位の描画データ（メッシュ・テクスチャ）。
/// AssetManagerが所有し、Modelは非所有ポインタとしてこれを参照する。
/// </summary>
class ModelData {
public:
    ModelData() = default;
    ~ModelData() = default;

    ModelData(const ModelData&) = delete;
    ModelData& operator=(const ModelData&) = delete;

    /// <summary>
    /// objファイルを読み込んでモデルデータを生成する
    /// </summary>
    /// <param name="directoryPath">objファイルが存在するディレクトリパス</param>
    /// <param name="filename">読み込むobjファイル名</param>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="commandList">テクスチャアップロードに使うコマンドリスト</param>
    /// <param name="srvHeap">SRV用ディスクリプタヒープ</param>
    /// <param name="descriptorSizeSRV">SRVディスクリプタ1個分のサイズ</param>
    /// <returns>生成したモデルデータ</returns>
    [[nodiscard]] static std::shared_ptr<ModelData> CreateFromObj(
        const std::string& directoryPath,
        const std::string& filename,
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12DescriptorHeap* srvHeap,
        uint32_t descriptorSizeSRV);

    /// <summary>
    /// 球メッシュのモデルデータを生成する（テクスチャなし）
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <returns>生成したモデルデータ</returns>
    [[nodiscard]] static std::shared_ptr<ModelData> CreateSphere(ID3D12Device* device);

    /// <summary>
    /// メッシュを取得する
    /// </summary>
    /// <returns>メッシュ</returns>
    const Mesh& GetMesh() const { return mesh_; }

    /// <summary>
    /// テクスチャハンドルを取得する
    /// </summary>
    /// <returns>テクスチャハンドル</returns>
    const TextureHandle& GetTexture() const { return texture_; }

private:
    Mesh mesh_;
    TextureHandle texture_;
};

} // namespace Engine

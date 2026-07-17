#pragma once
#include <d3d12.h>
#include <cstdint>
#include <memory>
#include <string>

#include "Model.h"
#include "ModelData.h"
#include "Sprite.h"
#include "TextureManager.h"

namespace Engine {

class DirectXCommon;

/// <summary>
/// DirectXCommonからdevice/commandList等を1度だけ受け取って保持し、
/// Model・Sprite・テクスチャの生成時に毎回引数で渡す手間を無くすクラス
/// </summary>
class AssetFactory {
public:
    AssetFactory() = default;
    ~AssetFactory() = default;

    /// <summary>
    /// 生成に必要な device / commandList 等を DirectXCommon から取得して保持する
    /// </summary>
    /// <param name="dx">描画基盤を保持するDirectXCommon</param>
    void Initialize(DirectXCommon* dx);

    /// <summary>
    /// objファイルを読み込んで、複数インスタンスで共有するモデルデータを生成する
    /// </summary>
    /// <param name="directoryPath">objファイルが存在するディレクトリパス</param>
    /// <param name="filename">読み込むobjファイル名</param>
    /// <returns>生成したモデルデータ</returns>
    [[nodiscard]] std::shared_ptr<ModelData> CreateModelData(const std::string& directoryPath, const std::string& filename);

    /// <summary>
    /// 球メッシュの、複数インスタンスで共有するモデルデータを生成する
    /// </summary>
    /// <returns>生成したモデルデータ</returns>
    [[nodiscard]] std::shared_ptr<ModelData> CreateSphereModelData();

    /// <summary>
    /// モデルデータを参照するモデルインスタンスを生成する
    /// </summary>
    /// <param name="modelData">参照する共有モデルデータ</param>
    /// <returns>生成したモデルインスタンス</returns>
    [[nodiscard]] std::unique_ptr<Model> CreateModelInstance(std::shared_ptr<const ModelData> modelData);

    /// <summary>
    /// 指定サイズの矩形スプライトを生成する
    /// </summary>
    /// <param name="width">スプライトの幅</param>
    /// <param name="height">スプライトの高さ</param>
    /// <returns>生成したスプライト</returns>
    [[nodiscard]] std::unique_ptr<Sprite> CreateSprite(float width, float height);

    /// <summary>
    /// テクスチャファイルを読み込み、GPUへ転送してSRVを作成しハンドルを返す
    /// </summary>
    /// <param name="filePath">読み込むテクスチャファイルのパス</param>
    /// <returns>読み込んだテクスチャのハンドル</returns>
    [[nodiscard]] TextureHandle LoadTexture(const std::string& filePath);

private:
    ID3D12Device* device_ = nullptr;
    ID3D12GraphicsCommandList* commandList_ = nullptr;
    ID3D12DescriptorHeap* srvHeap_ = nullptr;
    uint32_t descriptorSizeSRV_ = 0;
};

} // namespace Engine

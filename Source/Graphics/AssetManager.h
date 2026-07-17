#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "AssetFactory.h"
#include "Model.h"
#include "ModelData.h"

namespace Engine {

/// <summary>
/// resources以下のモデルをAssetFactoryを使って一括生成・管理し、
/// パス指定でモデルインスタンスを払い出すクラス
/// </summary>
class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager() = default;

    /// <summary>
    /// 登録済みモデルを全てAssetFactoryで読み込む
    /// </summary>
    /// <param name="factory">モデル生成に使うAssetFactory</param>
    void Initialize(AssetFactory* factory);

    /// <summary>
    /// 指定パスのモデルデータを参照する新しいモデルインスタンスを生成する
    /// </summary>
    /// <param name="path">RegisterModelで登録したパス（"ディレクトリ/ファイル名"）</param>
    /// <returns>生成したモデルインスタンス</returns>
    [[nodiscard]] std::unique_ptr<Model> CreateModel(const std::string& path);

    /// <summary>
    /// AssetManagerが持つモデルデータの参照を手放す。
    /// shared_ptrで共有しているため、Model側がまだ参照していれば実際の解放はそちらが破棄されるまで遅延される。
    /// </summary>
    void Clear();

private:
    /// <summary>
    /// 読み込むモデルのパスを登録し、AssetFactoryで読み込む
    /// </summary>
    /// <param name="directoryPath">objファイルが存在するディレクトリパス</param>
    /// <param name="filename">読み込むobjファイル名</param>
    void RegisterModel(const std::string& directoryPath, const std::string& filename);

private:
    AssetFactory* factory_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<ModelData>> modelData_;
};

} // namespace Engine

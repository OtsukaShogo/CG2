#pragma once
#include<cstdint>
#include <d3d12.h>
#include <wrl/client.h>
#include<string>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "TextureManager.h"

namespace Engine {

/// <summary>
/// マテリアル（色・ライティング有無・UV変換・テクスチャ）を管理するクラス
/// </summary>
class Material {
public:
    /// <summary>
    /// マテリアル用定数バッファのデータ構造
    /// </summary>
    struct ConstantData {
        Vector4   color          = { 1.0f, 1.0f, 1.0f, 1.0f };
        int32_t   enableLighting = 1;
        float     pad[3]         = {};  // float4x4 を 16byte 境界に揃えるためのパディング
        Matrix4x4 uvTransform   = {};
    };

public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    Material();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Material();

    /// <summary>
    /// マテリアル用の定数バッファリソースを生成する
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void Create(ID3D12Device* device);

    /// <summary>
    /// マテリアルの定数バッファとテクスチャをコマンドリストにバインドする
    /// </summary>
    /// <param name="commandList">バインド先のコマンドリスト</param>
    void Bind(ID3D12GraphicsCommandList* commandList);

    /// <summary>
    /// マテリアルテンプレートファイル(.mtl)を読み込み、テクスチャパスを取得する
    /// </summary>
    /// <param name="directoryPath">ファイルが存在するディレクトリパス</param>
    /// <param name="filename">読み込むファイル名</param>
    void LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

    /// <summary>
    /// 読み込んだテクスチャファイルパスを取得する
    /// </summary>
    /// <returns>テクスチャファイルパス</returns>
    const std::string& GetTextureFilePath() const { return textureFilePath_; }

    /// <summary>
    /// マテリアルの色を設定する
    /// </summary>
    /// <param name="color">設定する色</param>
    void SetColor(const Vector4& color);

    /// <summary>
    /// マテリアルの色を取得する（変更可能）
    /// </summary>
    /// <returns>マテリアルの色</returns>
    Vector4&   GetColor()       { return materialData_->color; }

    /// <summary>
    /// UV変換行列を取得する（変更可能）
    /// </summary>
    /// <returns>UV変換行列</returns>
    Matrix4x4& GetUVTransform() { return materialData_->uvTransform; }

    /// <summary>
    /// ライティングの有効・無効を設定する
    /// </summary>
    /// <param name="enable">ライティングを有効にする場合はtrue</param>
    void SetEnableLighting(bool enable) { materialData_->enableLighting = enable ? 1 : 0; }

    /// <summary>
    /// テクスチャのGPUディスクリプタハンドルを設定する
    /// </summary>
    /// <param name="handle">設定するGPUディスクリプタハンドル</param>
    void SetTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) { textureSrvHandleGPU_ = handle; }

    /// <summary>
    /// テクスチャを保持する（GPUへのアップロードが完了するまでtexture_/intermediateを生かしておく必要があるため）
    /// </summary>
    /// <param name="texture">保持するテクスチャハンドル</param>
    void SetTexture(const TextureHandle& texture) {
        texture_ = texture;
        textureSrvHandleGPU_ = texture.gpuHandle;
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    ConstantData* materialData_ = nullptr;

    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};

    std::string   textureFilePath_;
    TextureHandle texture_;
};

} // namespace Engine

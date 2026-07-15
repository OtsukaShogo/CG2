#pragma once
#include <wrl/client.h>
#include <dxcapi.h>
#include <string>
#include <unordered_map>

namespace Engine {

/// <summary>
/// DXCを使ったHLSLシェーダーのコンパイルとキャッシュを行うクラス
/// </summary>
class ShaderManager {
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    ShaderManager();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~ShaderManager();

    /// <summary>
    /// 指定したシェーダーファイルをコンパイルして返す（コンパイル済みの場合はキャッシュを返す）
    /// </summary>
    /// <param name="filePath">コンパイルするシェーダーファイルのパス</param>
    /// <param name="profile">シェーダーモデル（例: "vs_6_0"）</param>
    /// <returns>コンパイル済みシェーダーのバイナリ</returns>
    [[nodiscard]] IDxcBlob* Compile(const std::wstring& filePath,const wchar_t* profile);

    /// <summary>
    /// DXCコンパイラを初期化する
    /// </summary>
    void InitializeDXC();

private:
    bool initialized_ = false;

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

    // キャッシュ（同じファイルを何度もコンパイルしない）
    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<IDxcBlob>> shaderCache_;
};

} // namespace Engine

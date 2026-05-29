#pragma once
#include <wrl/client.h> 
#include <dxcapi.h>    
#include <string>
#include <unordered_map>

class ShaderManager {
public:

    ShaderManager();
    ~ShaderManager();

    // シェーダーをコンパイルして返す
    IDxcBlob* Compile(const std::wstring& filePath,const wchar_t* profile);

    void InitializeDXC();

private:
    bool initialized_ = false;

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

    // キャッシュ（同じファイルを何度もコンパイルしない）
    std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<IDxcBlob>> shaderCache_;
};


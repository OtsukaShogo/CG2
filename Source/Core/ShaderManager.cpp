#include "ShaderManager.h"
#include "DebugUtil.h"
#include <cassert>
#include <vector>

#pragma comment(lib, "dxcompiler.lib")

namespace Engine {

ShaderManager::ShaderManager(){}

ShaderManager::~ShaderManager(){}

// DXCコンパイラ一式（Utils/Compiler/IncludeHandler）を生成する。一度だけ初期化すればよい
void ShaderManager::InitializeDXC() {
    if (initialized_) return;

    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));

    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));

    initialized_ = true;
}

IDxcBlob* ShaderManager::Compile(const std::wstring& filePath,const wchar_t* profile){
    InitializeDXC();

    // キャッシュチェック
    std::wstring key = filePath + L"|" + profile;
    if (shaderCache_.contains(key)) {
        return shaderCache_[key].Get();
    }

    // ファイル読み込み
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
    HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &sourceBlob);
    assert(SUCCEEDED(hr));

    DxcBuffer sourceBuffer{};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = DXC_CP_UTF8;

    // コンパイル引数
    std::vector<LPCWSTR> arguments;
    arguments.push_back(filePath.c_str());
    arguments.push_back(L"-E");
    arguments.push_back(L"main");
    arguments.push_back(L"-T");
    arguments.push_back(profile);
    arguments.push_back(L"-Zi"); // デバッグ情報
    arguments.push_back(L"-Qembed_debug"); // デバッグ情報を埋め込む
    arguments.push_back(L"-Zpr"); // 行優先

    // コンパイル結果
    Microsoft::WRL::ComPtr<IDxcResult> result;
    hr = dxcCompiler_->Compile(
        &sourceBuffer,
        arguments.data(),
        (UINT)arguments.size(),
        includeHandler_.Get(),
        IID_PPV_ARGS(&result));
    assert(SUCCEEDED(hr));

    // エラー確認
    Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetStringLength() > 0) {
        DebugUtil::Log(errors->GetStringPointer());
        assert(false);
    }

    // 成功したバイナリを取得
    Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
    assert(SUCCEEDED(hr));

    // キャッシュに保存
    shaderCache_[key] = shaderBlob;

    return shaderBlob.Get();
}

} // namespace Engine

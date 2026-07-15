#include "D3D12Device.h"
#include <cassert>
#include <format>
#include <d3dcommon.h>
#include <d3d12sdklayers.h>

#include "DebugUtil.h"
#include "ConvertString.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Engine {

void D3D12Device::Initialize() {
    HRESULT hr = S_OK;

#ifdef _DEBUG
    {
        Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
            debugController->SetEnableGPUBasedValidation(TRUE);
        }
    }
#endif

    // DXGIファクトリ
    hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    // アダプタ選択
    // 性能優先順にアダプタを列挙し、ソフトウェアレンダリング（低速なフォールバックGPU）を除外して
    // 最初に見つかった実GPUを採用する
    for (UINT i = 0;
        dxgiFactory_->EnumAdapterByGpuPreference(
            i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(&adapter_)) != DXGI_ERROR_NOT_FOUND;
        ++i) {

        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = adapter_->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));

        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            DebugUtil::Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
            break;
        }
        adapter_.Reset();
    }
    assert(adapter_ != nullptr);

    // デバイス生成
    // 実行環境によって対応する最大機能レベルが異なるため、高い方から順に試して
    // 成功した最初のレベルでデバイスを生成する
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
    };
    const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        hr = D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
        if (SUCCEEDED(hr)) {
            DebugUtil::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
            break;
        }
    }
    assert(device_ != nullptr);
    DebugUtil::Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
    // InfoQueue 設定もここに移す
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
        };
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        infoQueue->PushStorageFilter(&filter);
    }
#endif
}

void D3D12Device::Finalize() {
    device_.Reset();
    adapter_.Reset();
    dxgiFactory_.Reset();
}

} // namespace Engine

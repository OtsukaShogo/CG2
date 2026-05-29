#pragma once
#include <d3d12.h>
#include <wrl/client.h> 
#include <string>
#include <unordered_map>

struct IDxcBlob;

class PipelineStateManager {
public:

    PipelineStateManager();
    ~PipelineStateManager();

    // 汎用的な GraphicsPipeline を作成してキャッシュする
    ID3D12PipelineState* CreateGraphicsPipeline(
        const std::string& name,
        ID3D12Device* device,
        IDxcBlob* vsBlob,
        IDxcBlob* psBlob,
        const D3D12_INPUT_LAYOUT_DESC& inputLayout,
        const D3D12_BLEND_DESC& blendDesc,
        const D3D12_RASTERIZER_DESC& rasterizerDesc,
        const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
        DXGI_FORMAT rtvFormat,
        DXGI_FORMAT dsvFormat);

    // 既に作った PSO を取得
    ID3D12PipelineState* GetPipelineState(const std::string& name) const;

    // 共通 RootSignature を取得
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

    // RootSignature を初期化（最初に一度だけ呼ぶ）
    void InitializeRootSignature(ID3D12Device* device);

    // PSO・RootSignatureをコマンドリストにセットする
    void SetPipeline(ID3D12GraphicsCommandList* commandList, const std::string& name);

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipelineStates_;
};

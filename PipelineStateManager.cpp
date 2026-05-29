#include "PipelineStateManager.h"
#include "DebugUtil.h"
#include <cassert>
#include<dxcapi.h>

PipelineStateManager::PipelineStateManager() {}

PipelineStateManager::~PipelineStateManager() {}

void PipelineStateManager::InitializeRootSignature(ID3D12Device* device) {
	if (rootSignature_) return;

	// ==== ここは今 WinMain に書いてある RootSignature の設定をほぼそのまま移植 ====

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// PixelShader 用 CBV (b0)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// VertexShader 用 CBV (b0)
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// SRV テーブル (t0)
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// Sampler
	D3D12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = &staticSampler;
	descriptionRootSignature.NumStaticSamplers = 1;

	// シリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			DebugUtil::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}

	// RootSignature 作成
	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

ID3D12PipelineState* PipelineStateManager::CreateGraphicsPipeline(
	const std::string& name,
	ID3D12Device* device,
	IDxcBlob* vsBlob,
	IDxcBlob* psBlob,
	const D3D12_INPUT_LAYOUT_DESC& inputLayout,
	const D3D12_BLEND_DESC& blendDesc,
	const D3D12_RASTERIZER_DESC& rasterizerDesc,
	const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	DXGI_FORMAT rtvFormat,
	DXGI_FORMAT dsvFormat)
{
	// 既にあるならそれを返す
	if (pipelineStates_.contains(name)) {
		return pipelineStates_[name].Get();
	}

	// RootSignature がまだなら作る
	InitializeRootSignature(device);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = rootSignature_.Get();
	desc.InputLayout = inputLayout;
	desc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	desc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
	desc.BlendState = blendDesc;
	desc.RasterizerState = rasterizerDesc;
	desc.DepthStencilState = depthStencilDesc;
	desc.DSVFormat = dsvFormat;

	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = rtvFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
	assert(SUCCEEDED(hr));

	pipelineStates_[name] = pso;
	return pso.Get();
}

ID3D12PipelineState* PipelineStateManager::GetPipelineState(const std::string& name) const {
	auto it = pipelineStates_.find(name);
	if (it == pipelineStates_.end()) {
		return nullptr;
	}
	return it->second.Get();
}

void PipelineStateManager::SetPipeline(ID3D12GraphicsCommandList* commandList, const std::string& name) {
	auto* pso = GetPipelineState(name);
	assert(pso != nullptr);
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pso);
}
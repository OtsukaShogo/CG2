#include "PipelineStateManager.h"
#include "DebugUtil.h"
#include "ShaderManager.h"
#include <cassert>
#include<dxcapi.h>

namespace Engine {

namespace {

// POSITION/TEXCOORD/NORMALを持つ標準的な頂点レイアウト（Object3D/Sprite共通）
// D3D12_INPUT_LAYOUT_DESCはポインタで要素配列を参照するため、
// 関数を抜けても参照が有効であるよう静的な配列として保持する
constexpr D3D12_INPUT_ELEMENT_DESC kStandardInputElements[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

/// <summary>
/// POSITION/TEXCOORD/NORMALを持つ標準的な頂点レイアウトを作成する
/// </summary>
D3D12_INPUT_LAYOUT_DESC MakeStandardInputLayout() {
	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.pInputElementDescs = kStandardInputElements;
	inputLayout.NumElements = _countof(kStandardInputElements);
	return inputLayout;
}

/// <summary>
/// 不透明描画用のブレンドステート（アルファブレンドなし、全チャンネル書き込み）を作成する
/// </summary>
D3D12_BLEND_DESC MakeOpaqueBlendDesc() {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	return blendDesc;
}

/// <summary>
/// 標準的なラスタライザステート（背面カリング・塗りつぶし）を作成する
/// </summary>
D3D12_RASTERIZER_DESC MakeStandardRasterizerDesc() {
	D3D12_RASTERIZER_DESC rasterDesc{};
	rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
	return rasterDesc;
}

} // namespace

PipelineStateManager::PipelineStateManager() {}

PipelineStateManager::~PipelineStateManager() {}

void PipelineStateManager::InitializeRootSignature(ID3D12Device* device) {
	if (rootSignature_) return;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_PARAMETER rootParameters[4] = {};
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

	// PixelShader 用 CBV (b1) - DirectionalLight
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

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

void PipelineStateManager::CreateGraphicsPipeline(
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
	// 既にあるなら何もしない
	if (pipelineStates_.contains(name)) {
		return;
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
}

ID3D12PipelineState* PipelineStateManager::GetPipelineState(const std::string& name) const {
	auto it = pipelineStates_.find(name);
	if (it == pipelineStates_.end()) {
		return nullptr;
	}
	return it->second.Get();
}

void PipelineStateManager::CreateObject3DPipeline(ID3D12Device* device, ShaderManager* shaderMgr) {
	IDxcBlob* vs = shaderMgr->Compile(L"Shaders/Object3d.VS.hlsl", L"vs_6_0");
	IDxcBlob* ps = shaderMgr->Compile(L"Shaders/Object3d.PS.hlsl", L"ps_6_0");

	D3D12_INPUT_LAYOUT_DESC inputLayout = MakeStandardInputLayout();
	D3D12_BLEND_DESC blendDesc = MakeOpaqueBlendDesc();
	D3D12_RASTERIZER_DESC rasterDesc = MakeStandardRasterizerDesc();

	D3D12_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	CreateGraphicsPipeline(
		kObject3D,
		device,
		vs,
		ps,
		inputLayout,
		blendDesc,
		rasterDesc,
		depthDesc,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_D24_UNORM_S8_UINT
	);
}

void PipelineStateManager::CreateSpritePipeline(ID3D12Device* device, ShaderManager* shaderMgr) {
	IDxcBlob* vs = shaderMgr->Compile(L"Shaders/Object3d.VS.hlsl", L"vs_6_0");
	IDxcBlob* ps = shaderMgr->Compile(L"Shaders/Object3d.PS.hlsl", L"ps_6_0");

	D3D12_INPUT_LAYOUT_DESC inputLayout = MakeStandardInputLayout();
	D3D12_BLEND_DESC blendDesc = MakeOpaqueBlendDesc();
	D3D12_RASTERIZER_DESC rasterDesc = MakeStandardRasterizerDesc();

	D3D12_DEPTH_STENCIL_DESC depthNone{};
	depthNone.DepthEnable = false;
	depthNone.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthNone.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	CreateGraphicsPipeline(
		kSprite,
		device,
		vs,
		ps,
		inputLayout,
		blendDesc,
		rasterDesc,
		depthNone,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_UNKNOWN
	);
}

void PipelineStateManager::SetPipeline(ID3D12GraphicsCommandList* commandList, const std::string& name) {
	auto* pso = GetPipelineState(name);
	assert(pso != nullptr);
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pso);
}

} // namespace Engine
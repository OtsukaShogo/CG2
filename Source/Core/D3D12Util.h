#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "externals/DirectXTex/DirectXTex.h"

namespace Engine {

/// <summary>
/// 指定サイズのアップロード用バッファリソースを生成する
/// </summary>
/// <param name="device">D3D12デバイス</param>
/// <param name="sizeInBytes">バッファサイズ（バイト単位）</param>
/// <returns>生成したバッファリソース</returns>
[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

/// <summary>
/// 指定した種類・数のディスクリプタヒープを生成する
/// </summary>
/// <param name="device">D3D12デバイス</param>
/// <param name="heapType">ディスクリプタヒープの種類</param>
/// <param name="numDescriptors">ディスクリプタ数</param>
/// <param name="shaderVisible">シェーダーから参照可能にするか</param>
/// <returns>生成したディスクリプタヒープ</returns>
[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

/// <summary>
/// テクスチャメタデータからテクスチャリソースを生成する
/// </summary>
/// <param name="device">D3D12デバイス</param>
/// <param name="metadata">テクスチャのメタデータ</param>
/// <returns>生成したテクスチャリソース</returns>
[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);

/// <summary>
/// ミップ画像データをテクスチャリソースへアップロードする
/// </summary>
/// <param name="texture">アップロード先のテクスチャリソース</param>
/// <param name="mipImages">アップロードするミップ画像データ</param>
/// <param name="device">D3D12デバイス</param>
/// <param name="commandList">コマンドリスト</param>
/// <returns>アップロード完了までコマンドリスト実行中に保持しておく中間リソース</returns>
[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

/// <summary>
/// 深度ステンシル用のテクスチャリソースを生成する
/// </summary>
/// <param name="device">D3D12デバイス</param>
/// <param name="width">テクスチャの幅</param>
/// <param name="height">テクスチャの高さ</param>
/// <returns>生成した深度ステンシルリソース</returns>
[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

/// <summary>
/// ディスクリプタヒープ内の指定インデックスのCPUディスクリプタハンドルを取得する
/// </summary>
/// <param name="descriptorHeap">対象のディスクリプタヒープ</param>
/// <param name="descriptorSize">ディスクリプタ1個分のサイズ</param>
/// <param name="index">取得するディスクリプタのインデックス</param>
/// <returns>指定インデックスのCPUディスクリプタハンドル</returns>
[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

/// <summary>
/// ディスクリプタヒープ内の指定インデックスのGPUディスクリプタハンドルを取得する
/// </summary>
/// <param name="descriptorHeap">対象のディスクリプタヒープ</param>
/// <param name="descriptorSize">ディスクリプタ1個分のサイズ</param>
/// <param name="index">取得するディスクリプタのインデックス</param>
/// <returns>指定インデックスのGPUディスクリプタハンドル</returns>
[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

} // namespace Engine

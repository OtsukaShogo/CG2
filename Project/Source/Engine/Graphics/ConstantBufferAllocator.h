#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Engine {

/// <summary>
/// 描画ごとに変わる定数バッファ（WVP・マテリアルなど）を、大きなアップロードバッファから切り出して渡すクラス。
/// オブジェクトごとにGPUリソースを作らずに済ませるためのもの。
/// 毎フレーム BeginFrame() で先頭に戻して使い回す。
/// </summary>
class ConstantBufferAllocator {
public:
	/// <summary>
	/// 切り出した領域（CPUから書き込み、GPUアドレスをルートパラメータへ渡す）
	/// </summary>
	struct Allocation {
		void* cpu = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS gpu = 0;
	};

	// 定数バッファの配置アライメント（D3D12の仕様で256バイト）
	static constexpr size_t kAlignment = 256;
	// 1ページの大きさ。足りなくなったら新しいページを追加する
	static constexpr size_t kPageSize = 1024 * 1024;

	ConstantBufferAllocator() = default;
	~ConstantBufferAllocator();

	ConstantBufferAllocator(const ConstantBufferAllocator&) = delete;
	ConstantBufferAllocator& operator=(const ConstantBufferAllocator&) = delete;

	/// <summary>
	/// 初期化する
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="frameCount">同時にGPUが使う可能性のあるフレーム数（今は1）</param>
	void Initialize(ID3D12Device* device, uint32_t frameCount = 1);

	/// <summary>
	/// 指定フレームの領域を先頭へ戻す。そのフレームのGPU処理が完了している必要がある
	/// </summary>
	/// <param name="frameIndex">フレーム番号（0〜frameCount-1）</param>
	void BeginFrame(uint32_t frameIndex = 0);

	/// <summary>
	/// 指定サイズの領域を切り出す（256バイト単位に切り上げる）。足りなければページを追加するので失敗しない
	/// </summary>
	/// <param name="size">必要なバイト数</param>
	[[nodiscard]] Allocation Allocate(size_t size);

	/// <summary>
	/// 値を書き込んだ領域を切り出して返す
	/// </summary>
	template <typename T>
	[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS Upload(const T& data) {
		Allocation a = Allocate(sizeof(T));
		*static_cast<T*>(a.cpu) = data;
		return a.gpu;
	}

	/// <summary>
	/// 解放する（GPUの処理が完了してから呼ぶこと）
	/// </summary>
	void Finalize();

	/// <summary>
	/// 現在のフレームで確保しているページ数（確認用）
	/// </summary>
	[[nodiscard]] size_t GetPageCount() const;

private:
	struct Page {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint8_t* cpu = nullptr;
		size_t size = 0;
	};

	struct FrameData {
		std::vector<Page> pages;
		size_t currentPage = 0;
		size_t offset = 0;
	};

	Page CreatePage(size_t size) const;

private:
	ID3D12Device* device_ = nullptr;
	std::vector<FrameData> frames_;
	uint32_t currentFrame_ = 0;
};

} // namespace Engine

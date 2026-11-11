#pragma once
#include <cstdint>
#include <vector>

namespace Engine {

/// <summary>
/// ディスクリプタヒープ内のインデックスを確保・解放するアロケータ。
/// 解放されたインデックスは再利用される（シーンの再読み込みなどでヒープが枯渇しないようにする）
/// </summary>
class DescriptorAllocator {
public:
	/// <summary>
	/// アロケータを初期化する（確保状態はすべてリセットされる）
	/// </summary>
	/// <param name="capacity">ヒープ全体のディスクリプタ数</param>
	/// <param name="firstIndex">確保を開始するインデックス（それより前は他の用途で予約済み）</param>
	void Initialize(uint32_t capacity, uint32_t firstIndex);

	/// <summary>
	/// 空きインデックスを1つ確保する。空きがない場合はログを出して停止する
	/// </summary>
	/// <returns>確保したインデックス</returns>
	[[nodiscard]] uint32_t Allocate();

	/// <summary>
	/// 確保済みのインデックスを解放して再利用できるようにする
	/// </summary>
	void Free(uint32_t index);

	/// <summary>
	/// 現在確保中のインデックス数を取得する
	/// </summary>
	[[nodiscard]] uint32_t GetUsedCount() const { return usedCount_; }

	/// <summary>
	/// 確保可能なインデックスの総数を取得する
	/// </summary>
	[[nodiscard]] uint32_t GetCapacity() const { return capacity_ - firstIndex_; }

private:
	uint32_t capacity_ = 0;
	uint32_t firstIndex_ = 0;
	uint32_t nextIndex_ = 0;          // まだ一度も使っていない最小のインデックス
	uint32_t usedCount_ = 0;
	std::vector<uint32_t> freeList_;  // 解放済みで再利用待ちのインデックス
	std::vector<bool> inUse_;         // 二重解放の検出用
};

} // namespace Engine

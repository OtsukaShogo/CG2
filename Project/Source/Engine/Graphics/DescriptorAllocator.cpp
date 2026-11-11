#include "DescriptorAllocator.h"
#include <cassert>
#include <cstdlib>
#include <string>

#include "../Core/DebugUtil.h"

namespace Engine {

void DescriptorAllocator::Initialize(uint32_t capacity, uint32_t firstIndex) {
	capacity_ = capacity;
	firstIndex_ = firstIndex;
	nextIndex_ = firstIndex;
	usedCount_ = 0;
	freeList_.clear();
	inUse_.assign(capacity, false);
}

uint32_t DescriptorAllocator::Allocate() {
	uint32_t index;
	if (!freeList_.empty()) {
		index = freeList_.back();
		freeList_.pop_back();
	} else if (nextIndex_ < capacity_) {
		index = nextIndex_++;
	} else {
		// Releaseビルドでも原因が分かるよう、必ずログを残して止める
		DebugUtil::Log("[DescriptorAllocator] SRVディスクリプタヒープが枯渇しました (capacity=" +
			std::to_string(capacity_) + ")\n");
		assert(false && "SRV descriptor heap exhausted");
		std::abort();
	}

	inUse_[index] = true;
	++usedCount_;
	return index;
}

void DescriptorAllocator::Free(uint32_t index) {
	// 範囲外・未確保・二重解放は無視する（Initialize後に古いハンドルが解放された場合など）
	if (index < firstIndex_ || index >= capacity_ || !inUse_[index]) {
		return;
	}
	inUse_[index] = false;
	--usedCount_;
	freeList_.push_back(index);
}

} // namespace Engine

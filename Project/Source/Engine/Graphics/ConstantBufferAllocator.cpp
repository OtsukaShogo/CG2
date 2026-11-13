#include "ConstantBufferAllocator.h"
#include "../Core/D3D12Util.h"
#include <algorithm>
#include <cassert>

namespace Engine {

ConstantBufferAllocator::~ConstantBufferAllocator() {
	Finalize();
}

void ConstantBufferAllocator::Initialize(ID3D12Device* device, uint32_t frameCount) {
	assert(frameCount >= 1);
	device_ = device;
	frames_.assign(frameCount, FrameData{});
	currentFrame_ = 0;
	for (FrameData& frame : frames_) {
		frame.pages.push_back(CreatePage(kPageSize));
	}
}

ConstantBufferAllocator::Page ConstantBufferAllocator::CreatePage(size_t size) const {
	Page page;
	page.resource = CreateBufferResource(device_, size);
	page.size = size;
	page.resource->Map(0, nullptr, reinterpret_cast<void**>(&page.cpu));
	return page;
}

void ConstantBufferAllocator::BeginFrame(uint32_t frameIndex) {
	assert(frameIndex < frames_.size());
	currentFrame_ = frameIndex;
	frames_[currentFrame_].currentPage = 0;
	frames_[currentFrame_].offset = 0;
}

ConstantBufferAllocator::Allocation ConstantBufferAllocator::Allocate(size_t size) {
	assert(!frames_.empty());
	FrameData& frame = frames_[currentFrame_];
	size_t alignedSize = (size + kAlignment - 1) & ~(kAlignment - 1);

	// 今のページに入らなければ、次のページ（なければ作る）へ進む
	while (frame.offset + alignedSize > frame.pages[frame.currentPage].size) {
		++frame.currentPage;
		frame.offset = 0;
		if (frame.currentPage == frame.pages.size()) {
			frame.pages.push_back(CreatePage((std::max)(kPageSize, alignedSize)));
		}
	}

	Page& page = frame.pages[frame.currentPage];
	Allocation allocation;
	allocation.cpu = page.cpu + frame.offset;
	allocation.gpu = page.resource->GetGPUVirtualAddress() + frame.offset;
	frame.offset += alignedSize;
	return allocation;
}

void ConstantBufferAllocator::Finalize() {
	for (FrameData& frame : frames_) {
		for (Page& page : frame.pages) {
			if (page.resource) {
				page.resource->Unmap(0, nullptr);
			}
		}
	}
	frames_.clear();
	device_ = nullptr;
}

size_t ConstantBufferAllocator::GetPageCount() const {
	return frames_.empty() ? 0 : frames_[currentFrame_].currentPage + 1;
}

} // namespace Engine

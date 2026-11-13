#include "CommandContext.h"
#include <cassert>

namespace Engine {

void CommandContext::Initialize(ID3D12Device* device) {
    HRESULT hr = S_OK;

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    for (auto& allocator : commandAllocators_) {
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
        assert(SUCCEEDED(hr));
    }

    hr = device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocators_[0].Get(), nullptr,
        IID_PPV_ARGS(&commandList_));
    assert(SUCCEEDED(hr));
}

void CommandContext::Reset(uint32_t frameIndex) {
    assert(frameIndex < kFrameCount);
    commandAllocators_[frameIndex]->Reset();
    commandList_->Reset(commandAllocators_[frameIndex].Get(), nullptr);
}

void CommandContext::CloseAndExecute() {
    commandList_->Close();
    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);
}

void CommandContext::Finalize() {
    commandList_.Reset();
    for (auto& allocator : commandAllocators_) {
        allocator.Reset();
    }
    commandQueue_.Reset();
}

} // namespace Engine

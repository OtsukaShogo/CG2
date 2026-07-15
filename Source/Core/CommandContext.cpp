#include "CommandContext.h"
#include <cassert>

namespace Engine {

void CommandContext::Initialize(ID3D12Device* device) {
    HRESULT hr = S_OK;

    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    hr = device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator_.Get(), nullptr,
        IID_PPV_ARGS(&commandList_));
    assert(SUCCEEDED(hr));
}

void CommandContext::Reset() {
    commandAllocator_->Reset();
    commandList_->Reset(commandAllocator_.Get(), nullptr);
}

void CommandContext::CloseAndExecute() {
    commandList_->Close();
    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);
}

void CommandContext::Finalize() {
    commandList_.Reset();
    commandAllocator_.Reset();
    commandQueue_.Reset();
}

} // namespace Engine

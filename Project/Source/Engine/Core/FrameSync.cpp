#include "FrameSync.h"
#include <cassert>

namespace Engine {

void FrameSync::Initialize(ID3D12Device* device) {
    HRESULT hr = device->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));

    fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent_ != nullptr);
}

uint64_t FrameSync::Signal(ID3D12CommandQueue* commandQueue) {
    fenceValue_++;
    commandQueue->Signal(fence_.Get(), fenceValue_);
    return fenceValue_;
}

void FrameSync::WaitFor(uint64_t fenceValue) {
    if (fence_->GetCompletedValue() < fenceValue) {
        fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void FrameSync::SignalAndWait(ID3D12CommandQueue* commandQueue) {
    // 単調増加のフェンスなので、最新の値を待てば、それ以前の処理もすべて完了している
    WaitFor(Signal(commandQueue));
}

void FrameSync::Finalize() {
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
    fence_.Reset();
}

} // namespace Engine

#include "FrameSync.h"
#include <cassert>

namespace Engine {

void FrameSync::Initialize(ID3D12Device* device) {
    HRESULT hr = device->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));

    fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent_ != nullptr);
}

void FrameSync::SignalAndWait(ID3D12CommandQueue* commandQueue) {
    // GPUがこのフレームのコマンドを実行し終えるまでCPU側をブロックし、
    // コマンドアロケータ・リストや解放待ちのリソースを安全に扱えるようにする
    fenceValue_++;
    commandQueue->Signal(fence_.Get(), fenceValue_);
    if (fence_->GetCompletedValue() < fenceValue_) {
        fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void FrameSync::Finalize() {
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
    fence_.Reset();
}

} // namespace Engine

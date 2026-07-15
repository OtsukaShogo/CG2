#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

namespace Engine {

/// <summary>
/// GPUフェンスによるCPU/GPU間の同期を管理するクラス
/// </summary>
class FrameSync {
public:
    FrameSync() = default;
    ~FrameSync() = default;

    FrameSync(const FrameSync&) = delete;
    FrameSync& operator=(const FrameSync&) = delete;

    /// <summary>
    /// フェンスと待機用イベントハンドルを生成する
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void Initialize(ID3D12Device* device);

    /// <summary>
    /// コマンドキューにフェンス値をシグナルし、GPUの完了をCPU側で待機する
    /// </summary>
    /// <param name="commandQueue">シグナル対象のコマンドキュー</param>
    void SignalAndWait(ID3D12CommandQueue* commandQueue);

    /// <summary>
    /// 終了処理を行う（イベントハンドルのクローズ・フェンスの解放）
    /// </summary>
    void Finalize();

private:
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;
};

} // namespace Engine

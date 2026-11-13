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
    /// コマンドキューにフェンス値をシグナルし、その値を返す（待たない）
    /// </summary>
    /// <param name="commandQueue">シグナル対象のコマンドキュー</param>
    /// <returns>このシグナルに対応するフェンス値。GPUがここまで到達したかをWaitForで待てる</returns>
    uint64_t Signal(ID3D12CommandQueue* commandQueue);

    /// <summary>
    /// 指定のフェンス値にGPUが到達するまで、CPU側で待機する（すでに到達していれば即座に戻る）
    /// </summary>
    void WaitFor(uint64_t fenceValue);

    /// <summary>
    /// コマンドキューにフェンス値をシグナルし、これまでの全GPU処理の完了をCPU側で待機する
    /// </summary>
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

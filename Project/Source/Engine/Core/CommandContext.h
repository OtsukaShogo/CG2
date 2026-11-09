#pragma once
#include <d3d12.h>
#include <wrl/client.h>

namespace Engine {

/// <summary>
/// コマンドキュー・コマンドアロケータ・コマンドリストの生成と、
/// 毎フレームのリセット／クローズ＆実行を担うクラス
/// </summary>
class CommandContext {
public:
    CommandContext() = default;
    ~CommandContext() = default;

    CommandContext(const CommandContext&) = delete;
    CommandContext& operator=(const CommandContext&) = delete;

    /// <summary>
    /// コマンドキュー・コマンドアロケータ・コマンドリストを生成する
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void Initialize(ID3D12Device* device);

    /// <summary>
    /// コマンドアロケータ・コマンドリストを次フレーム用にリセットする
    /// </summary>
    void Reset();

    /// <summary>
    /// コマンドリストを閉じてコマンドキューへ実行を投入する
    /// </summary>
    void CloseAndExecute();

    /// <summary>
    /// 終了処理を行う（コマンドリスト・アロケータ・キューの解放）
    /// </summary>
    void Finalize();

    /// <summary>
    /// コマンドリストを取得する
    /// </summary>
    /// <returns>コマンドリスト</returns>
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

    /// <summary>
    /// コマンドキューを取得する
    /// </summary>
    /// <returns>コマンドキュー</returns>
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
};

} // namespace Engine

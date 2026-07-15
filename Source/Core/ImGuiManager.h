#pragma once
#include <Windows.h>
#include <d3d12.h>

namespace Engine {

/// <summary>
/// ImGuiの初期化・フレーム制御・終了処理をまとめて扱うクラス
/// </summary>
class ImGuiManager {
public:
    /// <summary>
    /// シングルトンインスタンスを取得する
    /// </summary>
    /// <returns>ImGuiManagerのインスタンス</returns>
    [[nodiscard]] static ImGuiManager* GetInstance();

    //コピー禁止
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

    /// <summary>
    /// ImGuiを初期化する
    /// </summary>
    /// <param name="hwnd">対象のウィンドウハンドル</param>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="srvHeap">ImGuiが使用するSRVディスクリプタヒープ</param>
    void Initialize(HWND hwnd, ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);

    /// <summary>
    /// ImGuiの新しいフレームを開始する
    /// </summary>
    void BeginFrame();

    /// <summary>
    /// ImGuiの描画コマンドを構築してコマンドリストに積む
    /// </summary>
    /// <param name="commandList">描画コマンドを積むコマンドリスト</param>
    void EndFrame(ID3D12GraphicsCommandList* commandList);

    /// <summary>
    /// ImGuiの終了処理を行う
    /// </summary>
    void Finalize();

private:
    ImGuiManager() = default;
    ~ImGuiManager() = default;

private:
    bool initialized_ = false;
};

} // namespace Engine

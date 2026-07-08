#pragma once
#include <Windows.h>
#include <d3d12.h>

class ImGuiManager {
public:
    static ImGuiManager* GetInstance();

    //コピー禁止
    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

    void Initialize(HWND hwnd, ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);
    void BeginFrame();
    void EndFrame(ID3D12GraphicsCommandList* commandList);
    void Finalize();

private:
    ImGuiManager() = default;
    ~ImGuiManager() = default;

private:
    bool initialized_ = false;
};

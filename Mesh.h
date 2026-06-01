#pragma once
#include <vector>
#include <d3d12.h>
#include <wrl/client.h> 
#include "VertexData.h"

class Mesh {
public:
    Mesh();
    ~Mesh();

    // 球メッシュを生成
    void CreateSphere(uint32_t subdivision = 16);

    // 矩形メッシュを生成
    void CreateRect(float width, float height);

    void Upload();  // GPUへ頂点データを転送
    void Draw(ID3D12GraphicsCommandList* commandList);

    uint32_t GetVertexCount() const { return static_cast<uint32_t>(vertices_.size()); }

private:
    std::vector<VertexData> vertices_;
    std::vector<uint32_t>   indices_;

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vbv_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    D3D12_INDEX_BUFFER_VIEW ibv_{};
};
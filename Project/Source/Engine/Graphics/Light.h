#pragma once
#include <d3d12.h>
#include "../Math/Vector4.h"
#include "../Math/Vector3.h"
#include "ConstantBufferAllocator.h"

namespace Engine {

/// <summary>
/// 平行光源のデータを管理するクラス。
/// データはCPU側に持ち、描画時にアロケータへコピーして渡す（GPUが前のフレームを読んでいても書き換えられる）
/// </summary>
class Light {
public:
    /// <summary>
    /// 平行光源のパラメータ（色・方向・強度）
    /// </summary>
    struct DirectionalLight {
        Vector4 color     = { 1.0f, 1.0f, 1.0f, 1.0f };
        Vector3 direction = { 0.0f, -1.0f, 0.0f };
        float   intensity = 1.0f;
    };

    /// <summary>
    /// 平行光源データへのポインタを取得する（変更可能）
    /// </summary>
    /// <returns>平行光源データへのポインタ</returns>
    DirectionalLight* GetDirectionalLight() { return &directionalLight_; }

    /// <summary>
    /// 現在の平行光源データを、今フレームの定数バッファへ書き込み、そのGPU仮想アドレスを返す
    /// </summary>
    /// <param name="constantBuffers">定数バッファの切り出し元</param>
    /// <returns>平行光源定数バッファのGPU仮想アドレス</returns>
    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetDirectionalLightAddress(ConstantBufferAllocator& constantBuffers) const {
        return constantBuffers.Upload(directionalLight_);
    }

private:
    DirectionalLight directionalLight_;
};

} // namespace Engine

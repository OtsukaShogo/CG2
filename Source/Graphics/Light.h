#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "Vector4.h"
#include "Vector3.h"

namespace Engine {

/// <summary>
/// 平行光源の定数バッファを管理するクラス
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
    /// コンストラクタ
    /// </summary>
    Light();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Light();

    /// <summary>
    /// 平行光源用の定数バッファリソースを生成し初期化する
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    void Initialize(ID3D12Device* device);

    /// <summary>
    /// 平行光源データへのポインタを取得する（変更可能）
    /// </summary>
    /// <returns>平行光源データへのポインタ</returns>
    DirectionalLight* GetDirectionalLight() const { return directionalLightData_; }

    /// <summary>
    /// 平行光源定数バッファのGPU仮想アドレスを取得する
    /// </summary>
    /// <returns>平行光源定数バッファのGPU仮想アドレス</returns>
    D3D12_GPU_VIRTUAL_ADDRESS GetDirectionalLightAddress() const { return directionalLightResource_->GetGPUVirtualAddress(); }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
    DirectionalLight* directionalLightData_ = nullptr;
};

} // namespace Engine

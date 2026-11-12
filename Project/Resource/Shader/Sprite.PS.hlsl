#include "Object3d.hlsli"

// スプライト（2D描画・UI・スプライトフォント）専用のピクセルシェーダー。
// 3D用(Object3d.PS)と違い、アルファによる棄却（カットアウト）は行わない。
// 半透明・アンチエイリアスされた縁のピクセルは、ブレンドモードに従って合成される。

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    // スプライトはライティングしない
    output.color = gMaterial.color * textureColor;

    // 完全に透明なピクセルだけは破棄する（ブレンドなしのモードで黒く描かれるのを防ぐ）
    if (output.color.a == 0.0f)
    {
        discard;
    }

    return output;
}

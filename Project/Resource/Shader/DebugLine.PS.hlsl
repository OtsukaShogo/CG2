// デバッグ線描画用のピクセルシェーダー。頂点の色をそのまま出力する。

struct PixelShaderInput
{
    float32_t4 position : SV_POSITION;
    float32_t4 color : COLOR0;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.color = input.color;
    return output;
}

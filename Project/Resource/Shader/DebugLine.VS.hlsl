// デバッグ線描画用の頂点シェーダー。ワールド座標の頂点を、ビュープロジェクション行列で変換するだけ。

struct ViewProjection
{
    float32_t4x4 viewProjection;
};
ConstantBuffer<ViewProjection> gViewProjection : register(b0);

struct VertexShaderInput
{
    float32_t3 position : POSITION0;
    float32_t4 color : COLOR0;
};

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(float32_t4(input.position, 1.0f), gViewProjection.viewProjection);
    output.color = input.color;
    return output;
}

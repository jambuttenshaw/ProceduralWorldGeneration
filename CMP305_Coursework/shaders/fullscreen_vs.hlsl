
cbuffer CameraBuffer : register(b0)
{
    matrix projection;
    matrix invView;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD;
    float3 viewVector : POSITION0;
};

OutputType main(uint id : SV_VertexID)
{
    OutputType output;
 
    output.tex = float2(id / 2, id % 2);
    output.position = float4((output.tex.x - 0.5f) * 2.0f, (0.5f - output.tex.y) * 2.0f, 0.0f, 1.0f);
    
    float4 v = float4(
        output.position.x / projection._11,
        output.position.y / projection._22,
        1.0f,
        0.0f
    );
    output.viewVector = mul(v, invView);
    
    return output;
}
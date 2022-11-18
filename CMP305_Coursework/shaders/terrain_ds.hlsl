
Texture2D heightmap : register(t0);
SamplerState heightmapSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
}


struct DSOutput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : POSITION;
};

struct HSControlPointOutput
{
    float3 position : WORLD_SPACE_CONTROL_POINT_POSITION;
    float2 tex : CONTROL_POINT_TEXCOORD;
};

struct HSConstantOutput
{
    float edgeTessFactor[4] : SV_TessFactor;
    float insideTessFactor[2] : SV_InsideTessFactor;
};


float GetHeight(float2 pos)
{
    return heightmap.SampleLevel(heightmapSampler, pos, 0).r;
}


[domain("quad")]
DSOutput main(HSConstantOutput input, float2 domainUV : SV_DomainLocation, const OutputPatch<HSControlPointOutput, 4> patch)
{
	DSOutput output;

    output.worldPosition = lerp(lerp(patch[0].position, patch[1].position, domainUV.x),
                                lerp(patch[2].position, patch[3].position, domainUV.x),
                                domainUV.y);
    
    output.tex = lerp(lerp(patch[0].tex, patch[1].tex, domainUV.x),
                      lerp(patch[2].tex, patch[3].tex, domainUV.x),
                      domainUV.y);
    
    // apply displacement map
    output.worldPosition.y += GetHeight(output.tex);

    // transform vertex
    output.position = mul(float4(output.worldPosition, 1.0f), viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    return output;
}

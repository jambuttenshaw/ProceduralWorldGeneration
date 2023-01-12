#include "biomeHelper.hlsli"

Texture2D gBiomeMap : register(t0);
SamplerState gSampler : register(s0);

#define MAX_BIOMES 16

cbuffer BiomeColourBuffer : register(b0)
{
    float4 biomeColours[MAX_BIOMES];
    float2 worldMinPos;
    float2 worldSize;
};
cbuffer BiomeMappingBuffer : register(b1)
{
    BiomeMappingBuffer mappingBuffer;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
    const float3 borderColour = float3(1.0f, 0.0f, 0.0f);
    const float borderThickness = 0.01f;
    
    // flip y axis makes it easier to understand
    float2 uv = float2(input.tex.x, 1.0f - input.tex.y);
    
    float2 worldMin = GetBiomeMapUV(worldMinPos, mappingBuffer);
    float2 worldMax = GetBiomeMapUV(worldMinPos + worldSize, mappingBuffer);
    
    bool2 outerBounds = uv >= worldMin &&
                        uv <= worldMax;
    bool2 innerBounds = uv >= worldMin + borderThickness &&
                        uv <= worldMax - borderThickness;
    bool border = outerBounds.x && outerBounds.y && !(innerBounds.x && innerBounds.y);
    
    int biome = asint(gBiomeMap.Sample(gSampler, uv).r);
    float3 colour = border ? borderColour : biomeColours[biome].rgb;
    
    return float4(colour, 1.0f);
}

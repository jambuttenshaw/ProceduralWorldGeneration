#include "noiseFunctions.hlsli"
#include "biomeHelper.hlsli"

RWTexture2D<float4> gHeightmap : register(u0);

Texture2D<int> gBiomeMap : register(t0);
StructuredBuffer<TerrainNoiseSettings> gGenerationSettingsBuffer : register(t1);

SamplerState gBiomeMapSampler : register(s0);

cbuffer WorldBuffer : register(b0)
{
    float2 offset;
    float2 padding0;
}
cbuffer BiomeMappingBuffer : register(b1)
{
    BiomeMappingBuffer mappingBuffer;
}


[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 heightmapDims;
    gHeightmap.GetDimensions(heightmapDims.x, heightmapDims.y);
    
    if (dispatchThreadID.x > heightmapDims.x || dispatchThreadID.y > heightmapDims.y)
        return;
    
    float2 uv = float2(dispatchThreadID.xy) / float2(heightmapDims - float2(1, 1));
    float2 pos = uv + offset;
    
    uint2 biomeMapUV = GetBiomeMapLocation(pos, mappingBuffer);
    float2 noisePos = mappingBuffer.topLeft + pos;
    float2 biomeBlending = GetBiomeBlend(pos, mappingBuffer);
   
    
    float terrainHeight = 0.0f;
    if (length(biomeBlending) == 0.0f)
    {
        int biome = asint(gBiomeMap.Load(uint3(biomeMapUV, 0)).r);
        terrainHeight = TerrainNoise(noisePos, gGenerationSettingsBuffer[biome]);
    }
    else
    {
        int b1 = asint(gBiomeMap.Load(uint3(biomeMapUV + uint2(0, 0), 0)).r);
        int b2 = asint(gBiomeMap.Load(uint3(biomeMapUV + uint2(sign(biomeBlending.x), 0), 0)).r);
        int b3 = asint(gBiomeMap.Load(uint3(biomeMapUV + uint2(0, sign(biomeBlending.y)), 0)).r);
        int b4 = asint(gBiomeMap.Load(uint3(biomeMapUV + uint2(sign(biomeBlending.x), sign(biomeBlending.y)), 0)).r);
        
        float h1 = TerrainNoise(noisePos, gGenerationSettingsBuffer[b1]);
        float h2 = b2 == b1 ? h1 : TerrainNoise(noisePos, gGenerationSettingsBuffer[b2]);
        float h3 = b3 == b1 ? h1 : TerrainNoise(noisePos, gGenerationSettingsBuffer[b3]);
        float h4 = b4 == b1 ? h1 : TerrainNoise(noisePos, gGenerationSettingsBuffer[b4]);
        
        terrainHeight = lerp(
            lerp(h1, h3, abs(biomeBlending.y)),
            lerp(h2, h4, abs(biomeBlending.y)),
            abs(biomeBlending.x)
        );
    }
    
    float4 v = gHeightmap[dispatchThreadID.xy];
    v.r = terrainHeight;
    gHeightmap[dispatchThreadID.xy] = v;
}
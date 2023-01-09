#include "noiseFunctions.hlsli"
#include "math.hlsli"

RWTexture2D<float4> gHeightmap : register(u0);

cbuffer MatrixBuffer : register(b0)
{
    float2 offset;
    float2 padding;
}
cbuffer HeightmapSettingsBuffer : register(b1)
{
    SimpleNoiseSettings biomeNoiseSettings;
    // settings to generate the points
    int maxBiome;
    
    // settings to generate the voronoi
    float3 padding0;
    
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
    
    
    float biome = abs(SimpleNoise(pos, biomeNoiseSettings));
    biome = min(biome, maxBiome);
    
    float4 v = gHeightmap[dispatchThreadID.xy];
    v.g = frac(biome);
    gHeightmap[dispatchThreadID.xy] = v;
}

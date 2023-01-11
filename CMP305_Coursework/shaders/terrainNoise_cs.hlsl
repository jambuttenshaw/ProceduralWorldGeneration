#include "noiseFunctions.hlsli"
#include "math.hlsli"

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
    float2 biomeMapTopLeft;
    float biomeMapScale;
    uint biomeMapResolution;
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
    
    uint2 biomeMapUV = (biomeMapResolution - 1) * (pos - biomeMapTopLeft) / biomeMapScale;
    int biome = asint(gBiomeMap.Load(uint3(biomeMapUV, 0.0f)).r);
    TerrainNoiseSettings terrainSettings = gGenerationSettingsBuffer[biome];
    
    // apply warping
    pos += float2(SimpleNoise(pos + float2(17.13f, 23.7f), terrainSettings.warpSettings),
                 SimpleNoise(pos - float2(17.13f, 23.7f), terrainSettings.warpSettings));
    
    // create continent shape
    float continentShape = SimpleNoise(pos, terrainSettings.continentSettings);
    // create mountains
    float mountainShape = SmoothedRidgeNoise(pos, terrainSettings.mountainSettings);
    // mountains shouldn't stick out of the oceans as much
    float mountainMask = smoothstep(-terrainSettings.mountainBlend - terrainSettings.oceanFloorDepth, 0.0f, continentShape);
    
    // apply ocean floor
    continentShape = smoothMax(continentShape, -terrainSettings.oceanFloorDepth, terrainSettings.oceanFloorSmoothing);
    if (continentShape < 0)
        continentShape *= 1 + terrainSettings.oceanDepthMultiplier;
    
    float finalShape = continentShape + (mountainShape * mountainMask);
    
    float4 v = gHeightmap[dispatchThreadID.xy];
    v.r = finalShape;
    gHeightmap[dispatchThreadID.xy] = v;
}
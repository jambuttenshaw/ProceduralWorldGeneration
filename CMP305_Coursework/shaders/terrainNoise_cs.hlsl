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
    SimpleNoiseSettings warpSettings;
    SimpleNoiseSettings continentSettings;
    RidgeNoiseSettings mountainSettings;
    
    float oceanDepthMultiplier;
    float oceanFloorDepth;
    float oceanFloorSmoothing;
    float mountainBlend;
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
    
    // apply warping
    pos += float2(SimpleNoise(pos + float2(17.13f, 23.7f), warpSettings),
                 SimpleNoise(pos - float2(17.13f, 23.7f), warpSettings));
    
    // create continent shape
    float continentShape = SimpleNoise(pos, continentSettings);
    // create mountains
    float mountainShape = SmoothedRidgeNoise(pos, mountainSettings);
    // mountains shouldn't stick out of the oceans as much
    float mountainMask = smoothstep(-mountainBlend - oceanFloorDepth, 0.0f, continentShape);
    
    // apply ocean floor
    continentShape = smoothMax(continentShape, -oceanFloorDepth, oceanFloorSmoothing);
    if (continentShape < 0)
        continentShape *= 1 + oceanDepthMultiplier;
    
    float finalShape = continentShape + (mountainShape * mountainMask);
    
    gHeightmap[dispatchThreadID.xy] = finalShape;
}
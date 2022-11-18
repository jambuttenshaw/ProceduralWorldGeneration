#include "noiseFunctions.hlsli"

RWTexture2D<float4> gHeightmap : register(u0);

cbuffer HeightmapSettingsBuffer : register(b0)
{
    RidgeNoiseSettings settings;
}


[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 heightmapDims;
    gHeightmap.GetDimensions(heightmapDims.x, heightmapDims.y);
    
    if (dispatchThreadID.x > heightmapDims.x || dispatchThreadID.y > heightmapDims.y)
        return;
    
    float2 pos = float2(dispatchThreadID.xy) / float2(heightmapDims);
    
    float height = 0.0f;
    if (settings.peakSmoothing > 0.0f)
        height = SmoothedRidgeNoise(pos, settings);
    else
        height = RidgeNoise(pos, settings);
        
    gHeightmap[dispatchThreadID.xy] = height;
}
#include "noiseFunctions.hlsli"

RWTexture2D<float4> gHeightmap : register(u0);

cbuffer MatrixBuffer : register(b0)
{
    float4 params;
}
cbuffer HeightmapSettingsBuffer : register(b1)
{
    SimpleNoiseSettings settings;
}


[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 heightmapDims;
    gHeightmap.GetDimensions(heightmapDims.x, heightmapDims.y);
    
    if (dispatchThreadID.x > heightmapDims.x || dispatchThreadID.y > heightmapDims.y)
        return;
    
    float2 pos = float2(dispatchThreadID.xy) / float2(heightmapDims);
    
    gHeightmap[dispatchThreadID.xy] = SimpleNoise(pos, settings);
}
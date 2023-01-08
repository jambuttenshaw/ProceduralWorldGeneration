#include "noiseFunctions.hlsli"

RWTexture2D<float4> gHeightmap : register(u0);

cbuffer MatrixBuffer : register(b0)
{
    float4 params;
}
cbuffer HeightmapSettingsBuffer : register(b1)
{
    SimpleNoiseSettings warpSettings;
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
    pos += float2(SimpleNoise(pos + float2(17.13f, 23.7f), warpSettings),
                  SimpleNoise(pos - float2(17.13f, 23.7f), warpSettings));
    
    gHeightmap[dispatchThreadID.xy] = SimpleNoise(pos, settings);
}
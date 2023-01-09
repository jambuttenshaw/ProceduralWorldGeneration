#include "noiseFunctions.hlsli"
#include "math.hlsli"

struct BiomeType
{
	float2 centre;
	int type;
};

StructuredBuffer<BiomeType> gVoronoiPoints : register(t0);
RWTexture2D<float4> gHeightmap : register(u0);

cbuffer MatrixBuffer : register(b0)
{
    float2 offset;
    float2 padding;
}
cbuffer HeightmapSettingsBuffer : register(b1)
{
    float2 minPointBounds;
    float2 maxPointBounds;
    
	int pointCount;
	int biomeSeed;
	int numBiomeTypes;
	
    float padding0;
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
    
	float minDist = distance(gVoronoiPoints[0].centre, pos);
	int biomeType = gVoronoiPoints[0].type;
	for (int i = 1; i < pointCount; i++)
	{
		float dist = distance(gVoronoiPoints[i].centre, pos);
		if (dist < minDist)
		{
			minDist = dist;
			biomeType = gVoronoiPoints[i].type;
		}
	}
    
	float4 v = gHeightmap[dispatchThreadID.xy];
	v.g = float(biomeType) / float(numBiomeTypes);
	gHeightmap[dispatchThreadID.xy] = v;
}

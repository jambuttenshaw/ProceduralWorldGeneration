// Terrain Pixel Shader

#include "biomeHelper.hlsli"
#include "math.hlsli"
#include "noiseSimplex.hlsli"

Texture2D heightmap : register(t0);
Texture2D biomeMap : register(t1);
StructuredBuffer<BiomeTan> biomeTans : register(t2);

SamplerState heightmapSampler : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 diffuseColour;
	float4 ambientColour;
	float3 lightDirection;
	float padding0;
};

cbuffer TerrainBuffer : register(b1)
{
    float2 worldOffset;
    float2 padding1;
}
cbuffer BiomeMappingBuffer : register(b2)
{
    BiomeMappingBuffer mappingBuffer;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 worldPosition : POSITION;
};

float3 calculateNormal(float2 pos)
{
    const float gWorldCellSpace = 1 / 100.0f;
	
    const float gTexelCellSpaceU = 1.0f / 1024.0f;
    const float gTexelCellSpaceV = 1.0f / 1024.0f;
	
	// calculate normal from displacement map
    float2 leftTex = pos - float2(gTexelCellSpaceU, 0.0f);
    float2 rightTex = pos + float2(gTexelCellSpaceU, 0.0f);
    float2 topTex = pos - float2(0.0f, gTexelCellSpaceV);
    float2 bottomTex = pos + float2(0.0f, gTexelCellSpaceV);
	
    float thisY = heightmap.SampleLevel(heightmapSampler, pos, 0).r;
    float leftY = heightmap.SampleLevel(heightmapSampler, leftTex, 0).r;
    float rightY = heightmap.SampleLevel(heightmapSampler, rightTex, 0).r;
    float topY = heightmap.SampleLevel(heightmapSampler, topTex, 0).r;
    float bottomY = heightmap.SampleLevel(heightmapSampler, bottomTex, 0).r;
	
    // slightly better estimation of normals at the heightmap edge
    if (leftTex.x < 0.0f)
        leftY = 2.0f * thisY - rightY;
    if (rightTex.x > 1.0f)
        rightY = 2.0f * thisY - leftY;
    if (bottomTex.y > 1.0f)
        bottomY = 2.0f * thisY - topY;
    if (topTex.y < 0.0f)
        topY = 2.0f * thisY - bottomY;
    
    float3 tangent = normalize(float3(2.0f * gWorldCellSpace, rightY - leftY, 0.0f));
    float3 bitangent = normalize(float3(0.0f, bottomY - topY, -2.0f * gWorldCellSpace));
    return normalize(cross(tangent, bitangent));
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateDiffuse(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}

float4 main(InputType input) : SV_TARGET
{    
    float3 normal = calculateNormal(input.tex);
	
    // lighting:
    float4 lightColour = ambientColour + calculateDiffuse(-normalize(lightDirection), normal, diffuseColour);
    
    // biome:
    uint2 biomeMapUV = GetBiomeMapLocation(worldOffset + input.tex, mappingBuffer);
    float2 biomeBlend = GetBiomeBlend(worldOffset + input.tex, mappingBuffer);
    
    // blend biome tans
    BiomeTan biomeTan = BlendTans(biomeMap, biomeTans, biomeMapUV, biomeBlend);
    
    // steepness:
    // global up is always (0, 1, 0), so dot(normal, worldNormal) simplifies to normal.y
    float steepness = 1 - normal.y;
    
    float s1 = biomeTan.steepnessSmoothing * 0.5f;
    float s2 = biomeTan.heightSmoothing * 0.5f;
    float s3 = biomeTan.snowSmoothing * 0.5f;
    
    float steepnessStrength = 0.5f * (
        smoothstep(biomeTan.flatThreshold - s1, biomeTan.flatThreshold + s1, steepness) +
        smoothstep(biomeTan.cliffThreshold - s1, biomeTan.cliffThreshold + s1, steepness)
    );
    
    // biome colouring
    float3 groundColour;
    
    float detailNoise = smoothstep(biomeTan.detailThreshold - s1, biomeTan.detailThreshold + s1, remap01(snoise(2.0f * (worldOffset + input.tex))));
    float3 flatColour = lerp(biomeTan.flatDetailColour, biomeTan.flatColour, detailNoise);
    
    float shoreMix = 1 - smoothstep(biomeTan.shoreHeight - s2, biomeTan.shoreHeight + s2, input.worldPosition.y);
    groundColour = lerp(flatColour, biomeTan.shoreColour, shoreMix);
    
    
    // calculate final ground colour
    if (steepnessStrength < 0.5f)
        groundColour = lerp(groundColour, biomeTan.slopeColour, remap01(steepnessStrength, 0.0f, 0.5f));
    else
        groundColour = lerp(biomeTan.slopeColour, biomeTan.cliffColour, remap01(steepnessStrength, 0.5f, 1.0f));
    
    float snowMix = smoothstep(biomeTan.snowHeight - s2, biomeTan.snowHeight + s2, input.worldPosition.y) *
                    (1.0f - smoothstep(biomeTan.snowSteepness - s3, biomeTan.snowSteepness + s3, steepness));
    snowMix = sqrt(snowMix);
    groundColour = lerp(groundColour, biomeTan.snowColour, snowMix);
    
    return lightColour * float4(groundColour, 1.0f);
}




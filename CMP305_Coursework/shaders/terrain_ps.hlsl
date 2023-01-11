// Terrain Pixel Shader

#include "math.hlsli"

Texture2D heightmap : register(t0);
Texture2D biomeMap : register(t1);

SamplerState heightmapSampler : register(s0);
SamplerState biomeMapSampler : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 diffuseColour;
	float4 ambientColour;
	float3 lightDirection;
	float padding0;
};

cbuffer TerrainBuffer : register(b1)
{
    float flatThreshold;
    float cliffThreshold;
    float steepnessSmoothing;
    
    // mapping
    float biomeMapScale;
    float2 biomeMapTopLeft;
    float2 worldOffset;
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
    float2 biomeTex = (worldOffset + input.tex - biomeMapTopLeft) / biomeMapScale;
    int biome = asint(biomeMap.Sample(biomeMapSampler, biomeTex).r);
    float b = float(biome) / 6.0f;
    return float4(b.xxx, 1.0f);
    
    float3 normal = calculateNormal(input.tex);
	
    // lighting:
    float4 lightColour = ambientColour + calculateDiffuse(-normalize(lightDirection), normal, diffuseColour);
    
    // steepness:
    // global up is always (0, 1, 0), so dot(normal, worldNormal) simplifies to normal.y
    float steepness = 1 - normal.y;
    
    float transitionA = steepnessSmoothing * (1 - flatThreshold);
    float transitionB = steepnessSmoothing * (1 - cliffThreshold);
    
    float steepnessStrength = 0.5f * (
        smoothstep(flatThreshold - transitionA, flatThreshold + transitionA, steepness) +
        smoothstep(cliffThreshold - transitionB, cliffThreshold + transitionB, steepness)
    );
    
    // flat ground colouring/texturing
    float shoreMix = 1 - smoothstep(0, 1.5f, input.worldPosition.y);
    float3 shoreColour = float3(0.89f, 0.8f, 0.42f);
    float3 flatColour = float3(0.3f, 0.5f, 0.05f);
    flatColour = lerp(flatColour, shoreColour, shoreMix);
    
    
    // sloped ground colouring
    float3 slopeColour = float3(0.35f, 0.23f, 0.04f);
    
    
    // cliff ground colouring
    float3 cliffColour = float3(0.19f, 0.18f, 0.15f);
    
    
    // texturing:
	
    
    // calculate final ground colour
    float3 groundColour;
    if (steepnessStrength < 0.5f)
        groundColour = lerp(flatColour, slopeColour, remap01(steepnessStrength, 0.0f, 0.5f));
    else
        groundColour = lerp(slopeColour, cliffColour, remap01(steepnessStrength, 0.5f, 1.0f));
    
    return lightColour * float4(groundColour, 1.0f);
}




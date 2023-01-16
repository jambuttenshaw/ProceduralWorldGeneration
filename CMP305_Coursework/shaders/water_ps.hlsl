// Water post processing effect

#include "biomeHelper.hlsli"
#include "math.hlsli"


Texture2D renderTextureColour : register(t0);
Texture2D renderTextureDepth : register(t1);

Texture2D normalMapA : register(t2);
Texture2D normalMapB : register(t3);
SamplerState normalMapSampler : register(s0);

Texture2D biomeMap : register(t4);
StructuredBuffer<BiomeTan> biomeTans : register(t5);


cbuffer WaterBuffer : register(b0)
{
    // 64 bytes
    matrix projection;
    // 16 bytes
    float3 cameraPos;
    float depthMultiplier;
    // 16 bytes
    float3 oceanBoundsMin;
    float alphaMultiplier;
    // 16 bytes
    float3 oceanBoundsMax;
    float normalMapScale;
    // 16 bytes
    float normalMapStrength;
    float smoothness;
    float time;
    float padding0;
};

cbuffer LightBuffer : register(b1)
{
    float4 diffuseColour;
    float4 ambientColour;
    float4 specularColour;
    float3 lightDirection;
    float padding1;
};

cbuffer BiomeMappingBuffer : register(b2)
{
    BiomeMappingBuffer mappingBuffer;
};

struct InputType
{
    float4 position : SV_Position;
    float2 tex : TEXCOORD;
    float3 viewVector : POSITION0;
};


float3 normalMapToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    float3 normalT = (2.0f * normalMapSample) - 1.0f;
    
    // build orthonormal bases
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    
    return normalize(mul(normalT, TBN));
}


float calculateLinearDepth(float z)
{
    return projection._43 / (z - projection._33);
}


float4 main(InputType input) : SV_TARGET
{
    float4 colour = renderTextureColour[input.position.xy];
    
    float2 hitInfo = intersectAABB(cameraPos, input.viewVector, oceanBoundsMin, oceanBoundsMax);
    // the units of these distances are length(input.viewVector)
    // for some reason normalizing the view vector messes lots of stuff up?
    float distToOcean = hitInfo.x;
    float distThroughOcean = hitInfo.y;
    
    float depthToScene = calculateLinearDepth(renderTextureDepth[input.position.xy].r);

    if (distToOcean < distThroughOcean && distThroughOcean > 0 && distToOcean < depthToScene)
    {
        
        float depthThroughWater = min(depthToScene - distToOcean , distThroughOcean) ;
        
        float tDepth = 1 - exp(-depthMultiplier * depthThroughWater);
        float tAlpha = 1 - exp(-alphaMultiplier * depthThroughWater);
        
        // normal mapping
        
        // work out where on the ocean surface this fragment lies
        float3 intersectionPoint = cameraPos + input.viewVector * distToOcean;
        // find out the uv of the intersection point
        float2 uv = ((intersectionPoint - oceanBoundsMin) / (oceanBoundsMax - oceanBoundsMin)).xz;
        uv.y = 1 - uv.y;
        
        // this is not ideal
        const float3 normalW = float3(0, 1, 0);
        
        float3 normalMapASample = normalMapA.Sample(normalMapSampler, (uv * normalMapScale) + float2(0.1f * time, 0.08f * time)).xyz;
        float3 normalMapBSample = normalMapB.Sample(normalMapSampler, (uv * normalMapScale) + float2(-0.08f * time, -0.1f * time)).xyz;
        
        // convert normals to world space
        float3 bumpedNormal = normalMapToWorldSpace(normalMapASample, normalW, float3(1, 0, 0));
        bumpedNormal = 0.5f * (bumpedNormal + normalMapToWorldSpace(normalMapBSample, normalW, float3(1, 0, 0)));
        
        float3 n = lerp(normalW, bumpedNormal, normalMapStrength);
        
        // lighting
        float3 toLight = -normalize(lightDirection);
        
        float diffuse = saturate(dot(normalW, toLight));
        float4 lightColour = ambientColour + diffuse * diffuseColour;
        
        // guassian model of specular recflection
        float3 h = normalize(toLight - normalize(input.viewVector));
        float specularAngle = acos(saturate(dot(h, n)));
        float specularExponent = specularAngle / (1.0f - smoothness);
        float specularHighlight = exp(-specularExponent * specularExponent);
        float4 specular = saturate(specularHighlight * specularColour);
        
        // water colour
        
        // water colour depends on biome
        // biome:
        float2 pos = intersectionPoint.xz / 100.0f;
        uint2 biomeMapUV = GetBiomeMapLocation(pos, mappingBuffer);
        float2 biomeBlend = GetBiomeBlend(pos, mappingBuffer);
    
        // blend biome tans
        BiomeTan biomeTan = BlendTans(biomeMap, biomeTans, biomeMapUV, biomeBlend);
        
        float4 waterColour = float4(lerp(biomeTan.shallowWaterColour, biomeTan.deepWaterColour, tDepth), 1.0f);
        
        waterColour *= lightColour;
        waterColour += specular;
        
        colour = lerp(colour, waterColour, tAlpha);
    }
    
    return colour;
}

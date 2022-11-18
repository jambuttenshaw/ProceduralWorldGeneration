// Light pixel shader
// Calculate diffuse lighting for a single directional light

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float4 ambientColour;
    float3 lightDirection;
    float padding0;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateDiffuse(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    return saturate(diffuse * intensity);
}

float4 main(InputType input) : SV_TARGET
{
    float4 lightColour = ambientColour + calculateDiffuse(-normalize(lightDirection), input.normal, diffuseColour);
    return lightColour;
}

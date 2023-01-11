
Texture2D heightmap : register(t0);
SamplerState heightmapSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : POSITION;
};

float GetHeight(float2 pos)
{
    return heightmap.SampleLevel(heightmapSampler, pos, 0).r;
}

OutputType main(InputType input)
{
	OutputType output;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.worldPosition = mul(float4(input.position, 1.0f), worldMatrix).xyz;
    output.worldPosition.y += GetHeight(input.tex);
    
    output.position = mul(float4(output.worldPosition, 1.0f), viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	return output;
}
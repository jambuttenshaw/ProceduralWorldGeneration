
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
    float3 position : WORLD_SPACE_CONTROL_POINT_POSITION;
    float2 tex : CONTROL_POINT_TEXCOORD;
};


OutputType main(InputType input)
{
	OutputType output;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(float4(input.position, 1.0f), worldMatrix).xyz;

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	return output;
}
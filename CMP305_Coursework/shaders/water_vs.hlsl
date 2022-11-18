// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
};


OutputType main(InputType input)
{
    OutputType output;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    float4 worldSpacePos = mul(input.position, worldMatrix);
    float4 viewSpacePos = mul(worldSpacePos, viewMatrix);
    output.position = mul(viewSpacePos, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    output.worldPosition = mul(input.position, worldMatrix).xyz;
    
    return output;
}

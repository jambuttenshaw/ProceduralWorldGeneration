#pragma once

#include "BaseShader.h"
#include "BiomeGenerator.h"

using namespace DirectX;


class BiomeMapShader : public BaseShader
{
	struct BiomeColourBufferType
	{
		XMFLOAT4 biomeColours[MAX_BIOMES];
		XMFLOAT2 worldPos;
		int viewSize;
		float padding;
	};

public:
	BiomeMapShader(ID3D11Device* device, HWND hwnd);
	~BiomeMapShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, 
		BiomeGenerator* biomeGenerator, const XMFLOAT2& worldPos, int viewSize);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* m_MatrixBuffer = nullptr;
	ID3D11Buffer* m_BiomeColourBuffer = nullptr;
	ID3D11SamplerState* m_SampleState = nullptr;
};


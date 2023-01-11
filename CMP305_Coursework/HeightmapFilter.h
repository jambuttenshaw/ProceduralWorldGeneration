#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "nlohmann/json.hpp"

#include "Heightmap.h"
#include "BiomeGenerator.h"

using namespace DirectX;


class HeightmapFilter
{
	struct WorldBufferType
	{
		XMFLOAT2 offset;
		XMFLOAT2 padding;
	};

public:
	HeightmapFilter(ID3D11Device* device, const wchar_t* cs);
	~HeightmapFilter();

	void Run(ID3D11DeviceContext* deviceContext, Heightmap* heightmap, const BiomeGenerator* biomeGenerator);

protected:
	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3D11Buffer* m_WorldBuffer = nullptr;
};

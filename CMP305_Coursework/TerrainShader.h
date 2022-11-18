#pragma once

#include "DXF.h"
#include <nlohmann/json.hpp>

using namespace std;
using namespace DirectX;


class TerrainShader
{
private:
	struct VSMatrixBufferType
	{
		XMMATRIX world;
	};
	struct DSMatrixBufferType
	{
		XMMATRIX view;
		XMMATRIX projection;
	};
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT4 ambient;
		XMFLOAT3 direction;
		float padding;
	};
	struct TerrainBufferType
	{
		float flatThreshold;
		float cliffThreshold;
		float steepnessSmoothing;
		float padding;
	};
	struct TessellationBufferType
	{
		XMFLOAT2 minMaxDistance;
		XMFLOAT2 minMaxLOD;
		XMFLOAT3 cameraPos;
		float padding;
	};

public:
	TerrainShader(ID3D11Device* device);
	~TerrainShader();

	void SetShaderParameters(ID3D11DeviceContext* deviceContext,
								const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection,
								ID3D11ShaderResourceView* heightmap, Light* light, Camera* camera);
	void Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount);

	void GUI();

	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);

private:
	void InitShader();
	
	void LoadVS(const wchar_t* vs);
	void LoadHS(const wchar_t* hs);
	void LoadDS(const wchar_t* ds);
	void LoadPS(const wchar_t* ps);

	void CreateBuffer(UINT byteWidth, ID3D11Buffer** ppBuffer);

private:
	ID3D11Device* m_Device = nullptr;

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11HullShader* m_HullShader = nullptr;
	ID3D11DomainShader* m_DomainShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	ID3D11InputLayout* m_InputLayout = nullptr;

	ID3D11Buffer* m_VSMatrixBuffer = nullptr;		// matrices to be sent to vertex shader
	ID3D11Buffer* m_TessellationBuffer = nullptr;	// tessellation parameters
	ID3D11Buffer* m_DSMatrixBuffer = nullptr;		// matrices to be sent to domain shader
	ID3D11Buffer* m_LightBuffer = nullptr;			// lighting data
	ID3D11Buffer* m_TerrainBuffer = nullptr;		// terrain data

	ID3D11SamplerState* m_HeightmapSampleState = nullptr;

	// terrain properties
	float m_FlatThreshold = 0.5f;
	float m_CliffThreshold = 0.8f;
	float m_SteepnessSmoothing = 0.1f;

	// tessellation params
	XMFLOAT2 m_MinMaxDistance{ 0.0f, 50.0f };
	XMFLOAT2 m_MinMaxLOD{ 1.0f, 16.0f };
};


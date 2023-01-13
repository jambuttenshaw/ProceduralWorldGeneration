#include "TerrainShader.h"

#include "Heightmap.h"
#include "BiomeGenerator.h"

TerrainShader::TerrainShader(ID3D11Device* device)
	: m_Device(device)
{
	InitShader();
}


TerrainShader::~TerrainShader()
{
	if (m_VertexShader) m_VertexShader->Release();
	if (m_PixelShader) m_PixelShader->Release();

	if (m_InputLayout) m_InputLayout->Release();
	
	if (m_MatrixBuffer) m_MatrixBuffer->Release();
	if (m_LightBuffer) m_LightBuffer->Release();
	if (m_WorldBuffer) m_WorldBuffer->Release();

	if (m_HeightmapSampleState) m_HeightmapSampleState->Release();
}

void TerrainShader::InitShader()
{
	// load and compile shaders
	LoadVS(L"terrain_vs.cso");
	LoadPS(L"terrain_ps.cso");

	// create constant buffers
	CreateBuffer(sizeof(MatrixBufferType), &m_MatrixBuffer);
	CreateBuffer(sizeof(LightBufferType), &m_LightBuffer);
	CreateBuffer(sizeof(WorldBufferType), &m_WorldBuffer);

	// create sampler state
	D3D11_SAMPLER_DESC heightmapSamplerDesc;
	heightmapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	heightmapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.MipLODBias = 0.0f;
	heightmapSamplerDesc.MaxAnisotropy = 1;
	heightmapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	heightmapSamplerDesc.MinLOD = 0;
	heightmapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&heightmapSamplerDesc, &m_HeightmapSampleState);
}

void TerrainShader::LoadVS(const wchar_t* vs)
{
	ID3DBlob* vertexShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(vs, &vertexShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the vertex shader from the buffer.
	m_Device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_VertexShader);

	// Create the vertex input layout description.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Get a count of the elements in the layout.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	m_Device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_InputLayout);

	vertexShaderBuffer->Release();
}

void TerrainShader::LoadPS(const wchar_t* ps)
{
	ID3DBlob* pixelShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(ps, &pixelShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_PixelShader);
	pixelShaderBuffer->Release();
}

void TerrainShader::CreateBuffer(UINT byteWidth, ID3D11Buffer** ppBuffer)
{
	assert(byteWidth % 16 == 0 && "Constant buffer byte width must be multiple of 16!");

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	m_Device->CreateBuffer(&desc, NULL, ppBuffer);
}


void TerrainShader::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
	Heightmap* heightmap, const BiomeGenerator* biomeGenerator,
	Light* light)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// update data in buffers
	{
		result = deviceContext->Map(m_MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
		dataPtr->world = tworld;
		dataPtr->view = tview;
		dataPtr->projection = tproj;
		deviceContext->Unmap(m_MatrixBuffer, 0);
	}
	{
		deviceContext->Map(m_LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		LightBufferType* dataPtr = (LightBufferType*)mappedResource.pData;
		dataPtr->diffuse = light->getDiffuseColour();
		dataPtr->ambient = light->getAmbientColour();
		dataPtr->direction = light->getDirection();
		dataPtr->padding = 0.0f;
		deviceContext->Unmap(m_LightBuffer, 0);
	}
	{
		deviceContext->Map(m_WorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		WorldBufferType* dataPtr = (WorldBufferType*)mappedResource.pData;
		dataPtr->worldOffset = heightmap->GetOffset();
		deviceContext->Unmap(m_WorldBuffer, 0);
	}

	deviceContext->VSSetConstantBuffers(0, 1, &m_MatrixBuffer);
	ID3D11ShaderResourceView* heightmapSRV = heightmap->GetSRV();
	deviceContext->VSSetShaderResources(0, 1, &heightmapSRV);
	deviceContext->VSSetSamplers(0, 1, &m_HeightmapSampleState);

	ID3D11Buffer* psCBs[] = { m_LightBuffer, m_WorldBuffer, biomeGenerator->GetBiomeMappingBuffer() };
	deviceContext->PSSetConstantBuffers(0, 3, psCBs);
	ID3D11ShaderResourceView* psSRVs[] = { heightmapSRV, biomeGenerator->GetBiomeMapSRV(), biomeGenerator->GetBiomeTanningSRV() };
	deviceContext->PSSetShaderResources(0, 3, psSRVs);
	deviceContext->PSSetSamplers(0, 1, &m_HeightmapSampleState);
}

void TerrainShader::Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount)
{
	deviceContext->IASetInputLayout(m_InputLayout);

	deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_PixelShader, nullptr, 0);

	deviceContext->DrawIndexed(indexCount, 0, 0);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);
}

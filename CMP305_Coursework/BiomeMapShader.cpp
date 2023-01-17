#include "BiomeMapShader.h"

#include "BiomeGenerator.h"


BiomeMapShader::BiomeMapShader(ID3D11Device* device, HWND hwnd) 
	: BaseShader(device, hwnd)
{
	initShader(L"biomeMap_vs.cso", L"biomeMap_ps.cso");
}

BiomeMapShader::~BiomeMapShader()
{
	if (m_SampleState) m_SampleState->Release();
	if (m_MatrixBuffer) m_MatrixBuffer->Release();
	if (layout) layout->Release();
}


void BiomeMapShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(MatrixBufferType);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT hr = renderer->CreateBuffer(&bufferDesc, NULL, &m_MatrixBuffer);
	assert(hr == S_OK);

	bufferDesc.ByteWidth = sizeof(BiomeColourBufferType);
	hr = renderer->CreateBuffer(&bufferDesc, NULL, &m_BiomeColourBuffer);
	assert(hr == S_OK);

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = renderer->CreateSamplerState(&samplerDesc, &m_SampleState);
	assert(hr == S_OK);
}


void BiomeMapShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, 
	BiomeGenerator* biomeGenerator, const XMFLOAT2& worldPos, int viewSize)
{
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr = deviceContext->Map(m_MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(hr == S_OK);
		MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
		dataPtr->world = XMMatrixTranspose(worldMatrix);
		dataPtr->view = XMMatrixTranspose(viewMatrix);
		dataPtr->projection = XMMatrixTranspose(projectionMatrix);
		deviceContext->Unmap(m_MatrixBuffer, 0);
	}
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr = deviceContext->Map(m_BiomeColourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(hr == S_OK);
		BiomeColourBufferType* dataPtr = (BiomeColourBufferType*)mappedResource.pData;
		memcpy(dataPtr->biomeColours, biomeGenerator->GetBiomeMinimapColours(), sizeof(XMFLOAT4) * MAX_BIOMES);
		dataPtr->worldPos = worldPos;
		dataPtr->viewSize = viewSize;
		deviceContext->Unmap(m_BiomeColourBuffer, 0);
	}

	deviceContext->VSSetConstantBuffers(0, 1, &m_MatrixBuffer);
	ID3D11Buffer* psCBs[] = { m_BiomeColourBuffer, biomeGenerator->GetBiomeMappingBuffer() };
	deviceContext->PSSetConstantBuffers(0, 2, psCBs);

	// Set shader texture and sampler resource in the pixel shader.
	ID3D11ShaderResourceView* biomeMap = biomeGenerator->GetBiomeMapSRV();
	deviceContext->PSSetShaderResources(0, 1, &biomeMap);
	deviceContext->PSSetSamplers(0, 1, &m_SampleState);
}

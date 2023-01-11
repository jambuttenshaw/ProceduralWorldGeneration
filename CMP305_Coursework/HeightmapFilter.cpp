#include "HeightmapFilter.h"


HeightmapFilter::HeightmapFilter(ID3D11Device* device, const wchar_t* cs)
{
	HRESULT hr;

	// load compute shader from file
	ID3D10Blob* computeShaderBuffer;

	// Reads compiled shader into buffer (bytecode).
	hr = D3DReadFileToBlob(cs, &computeShaderBuffer);
	assert(hr == S_OK && "Failed to load shader");

	// Create the compute shader from the buffer.
	hr = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &m_ComputeShader);
	assert(hr == S_OK);
	computeShaderBuffer->Release();

	// Setup description of heightmap settings constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(WorldBufferType);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, NULL, &m_WorldBuffer);
	assert(hr == S_OK);
}

HeightmapFilter::~HeightmapFilter()
{
	if (m_ComputeShader) m_ComputeShader->Release();
	if (m_WorldBuffer) m_WorldBuffer->Release();
}

void HeightmapFilter::Run(ID3D11DeviceContext* deviceContext, Heightmap* heightmap, const BiomeGenerator* biomeGenerator)
{
	ID3D11UnorderedAccessView* uav = heightmap->GetUAV();
	deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
	ID3D11ShaderResourceView* srvs[2] = { biomeGenerator->GetBiomeMapSRV(), biomeGenerator->GetGenerationSettingsSRV() };
	deviceContext->CSSetShaderResources(0, 2, srvs);

	// update data in constant buffers
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->Map(m_WorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WorldBufferType* dataPtr = reinterpret_cast<WorldBufferType*>(mappedResource.pData);
	dataPtr->offset = heightmap->GetOffset();
	deviceContext->Unmap(m_WorldBuffer, 0);

	ID3D11Buffer* cscbs[2] = { m_WorldBuffer, biomeGenerator->GetBiomeMappingBuffer() };
	deviceContext->CSSetConstantBuffers(0, 2, cscbs);

	deviceContext->CSSetShader(m_ComputeShader, nullptr, 0);

	// assume thread groups consist of 16x16x1 threads
	unsigned int groupCount = (heightmap->GetResolution() + 15) / 16; // (fast ceiling of integer division)
	deviceContext->Dispatch(groupCount, groupCount, 1);

	// clean up
	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	deviceContext->CSSetShaderResources(0, 2, nullSRVs);
	ID3D11Buffer* nullCBs[3] = { nullptr, nullptr, nullptr };
	deviceContext->CSSetConstantBuffers(0, 3, nullCBs);
}

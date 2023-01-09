#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "nlohmann/json.hpp"

#include "Heightmap.h"

using namespace DirectX;


class IHeightmapFilter
{
public:
	virtual ~IHeightmapFilter() = default;

	// pure virtual methods for BaseHeightmapFilter to implemente
	virtual void Run(ID3D11DeviceContext* deviceContext, Heightmap* heightmap) = 0;
	virtual bool SettingsGUI() = 0;
	virtual nlohmann::json Serialize() const = 0;
	virtual void LoadFromJson(const nlohmann::json& data) = 0;

	// pure virtual methods for heightmap filters to implement
	virtual const char* Label() const = 0;
};

/*
* Handles interaction with DirectX and compute shader
* 
* IMPORTANT
* SettingsType MUST be correctly padded in groups of 16 bytes!!!
* SettingsType objects are used DIRECTLY as contents of DirectX Constant Buffers!!!
*/
template <typename SettingsType>
class BaseHeightmapFilter : public IHeightmapFilter
{
	struct MatrixBufferType
	{
		XMFLOAT2 offset;
		XMFLOAT2 padding;
	};

public:
	BaseHeightmapFilter(ID3D11Device* device, const wchar_t* cs)
		: m_Device(device)
	{
		HRESULT hr;
		assert(sizeof(SettingsType) % 16 == 0);

		// load compute shader from file
		ID3D10Blob* computeShaderBuffer;

		// Reads compiled shader into buffer (bytecode).
		hr = D3DReadFileToBlob(cs, &computeShaderBuffer);
		assert(hr == S_OK && "Failed to load shader");

		// Create the compute shader from the buffer.
		hr = m_Device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &m_ComputeShader);
		assert(hr == S_OK);
		computeShaderBuffer->Release();

		// Setup description of heightmap settings constant buffer
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(SettingsType);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		hr = m_Device->CreateBuffer(&bufferDesc, NULL, &m_SettingsBuffer);
		assert(hr == S_OK);

		bufferDesc.ByteWidth = sizeof(MatrixBufferType);
		hr = m_Device->CreateBuffer(&bufferDesc, NULL, &m_MatrixBuffer);
		assert(hr == S_OK);
	}

	virtual ~BaseHeightmapFilter()
	{
		if (m_ComputeShader) m_ComputeShader->Release();
		if (m_MatrixBuffer) m_SettingsBuffer->Release();
		if (m_SettingsBuffer) m_SettingsBuffer->Release();
	}

	virtual void Run(ID3D11DeviceContext* deviceContext, Heightmap* heightmap) override
	{
		ID3D11UnorderedAccessView* uav = heightmap->GetUAV();
		deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

		// update data in constant buffers
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		deviceContext->Map(m_MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		MatrixBufferType* dataPtr = reinterpret_cast<MatrixBufferType*>(mappedResource.pData);
		dataPtr->offset = heightmap->GetOffset();
		deviceContext->Unmap(m_MatrixBuffer, 0);

		deviceContext->Map(m_SettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &m_Settings, sizeof(m_Settings));
		deviceContext->Unmap(m_SettingsBuffer, 0);

		ID3D11Buffer* cscbs[2] = { m_MatrixBuffer, m_SettingsBuffer };
		deviceContext->CSSetConstantBuffers(0, 2, cscbs);

		deviceContext->CSSetShader(m_ComputeShader, nullptr, 0);

		// assume thread groups consist of 16x16x1 threads
		unsigned int groupCount = (heightmap->GetResolution() + 15) / 16; // (fast ceiling of integer division)
		deviceContext->Dispatch(groupCount, groupCount, 1);

		deviceContext->CSSetShader(nullptr, nullptr, 0);

		ID3D11UnorderedAccessView* nullUAV = nullptr;
		deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		ID3D11Buffer* nullCB = nullptr;
		deviceContext->CSSetConstantBuffers(0, 1, &nullCB);
	}

	virtual bool SettingsGUI() override
	{
		return m_Settings.SettingsGUI();
	}

	virtual nlohmann::json Serialize() const override
	{
		nlohmann::json serialized = m_Settings.Serialize();
		serialized["name"] = Label();

		return serialized;
	}
	virtual void LoadFromJson(const nlohmann::json& data) override
	{
		m_Settings.LoadFromJson(data);
	}

protected:
	ID3D11Device* m_Device;

	ID3D11ComputeShader* m_ComputeShader = nullptr;
	
	ID3D11Buffer* m_MatrixBuffer = nullptr;
	ID3D11Buffer* m_SettingsBuffer = nullptr;
	SettingsType m_Settings;
};

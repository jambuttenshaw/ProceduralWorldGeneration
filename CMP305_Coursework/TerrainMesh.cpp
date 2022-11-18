#include "TerrainMesh.h"

#define clamp(v, minimum, maximum) (max(min(v, maximum), minimum))


TerrainMesh::TerrainMesh(ID3D11Device* device)
	: TerrainMesh(device, 50, 100.0f)
{
}

TerrainMesh::TerrainMesh(ID3D11Device* device, unsigned int resolution, float size)
{
	BuildMesh(device, resolution, size);
	CreateHeightmapTexture(device);
}

TerrainMesh::~TerrainMesh()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_IndexBuffer) m_IndexBuffer->Release();

	// Cleanup the heightMap
	if (m_UAV) m_UAV->Release();
	if (m_SRV) m_SRV->Release();
}

void TerrainMesh::SendData(ID3D11DeviceContext* deviceContext)
{
	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);
}

void TerrainMesh::BuildMesh(ID3D11Device* device, unsigned int resolution, float size)
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_IndexBuffer) m_IndexBuffer->Release();

	m_Resolution = resolution;
	m_Size = size;

	m_VertexCount = (resolution + 1) * (resolution + 1);
	m_IndexCount = 12 * resolution * resolution;

	VertexType* vertices = new VertexType[m_VertexCount];
	unsigned long* indices = new unsigned long[m_IndexCount];

	float fResolution = static_cast<float>(resolution);

	for (unsigned int x = 0; x < resolution + 1; x++)
	{
		for (unsigned int z = 0; z < resolution + 1; z++)
		{
			float fX = static_cast<float>(x) / fResolution - 0.5f;
			float fZ = static_cast<float>(z) / fResolution - 0.5f;

			VertexType v;
			v.Position = { size * fX, 0.0f, size * fZ };
			v.UV = { fX + 0.5f, fZ + 0.5f };
			v.Normal = { 0.0f, 1.0f, 0.0f };

			vertices[x + z * (resolution + 1)] = v;
		}
	}

	unsigned int i = 0;
	for (unsigned int x = 0; x < resolution; x++)
	{
		for (unsigned int z = 0; z < resolution; z++)
		{

			// Define 12 control points per terrain quad

			// 0-3 are the actual quad vertices
			indices[i + 0] = (z + 0) + (x + 0) * (resolution + 1);
			indices[i + 1] = (z + 1) + (x + 0) * (resolution + 1);
			indices[i + 2] = (z + 0) + (x + 1) * (resolution + 1);
			indices[i + 3] = (z + 1) + (x + 1) * (resolution + 1);

			// 4-5 are +x   
			indices[i + 4] = clamp(z + 0, 0, resolution) + clamp(x + 2, 0, resolution) * (resolution + 1);
			indices[i + 5] = clamp(z + 0, 0, resolution) + clamp(x + 2, 0, resolution) * (resolution + 1);

			// 6-7 are +z   
			indices[i + 6] = clamp(z + 2, 0, resolution) + clamp(x + 0, 0, resolution) * (resolution + 1);
			indices[i + 7] = clamp(z + 2, 0, resolution) + clamp(x + 0, 0, resolution) * (resolution + 1);

			// 8-9 are -x   
			indices[i + 8] = clamp(z + 0, 0, resolution) + clamp(x - 1, 0, resolution) * (resolution + 1);
			indices[i + 9] = clamp(z + 0, 0, resolution) + clamp(x - 1, 0, resolution) * (resolution + 1);

			// 10-11 are -z
			indices[i + 10] = clamp(z - 1, 0, resolution) + clamp(x + 0, 0, resolution) * (resolution + 1);
			indices[i + 11] = clamp(z - 1, 0, resolution) + clamp(x + 0, 0, resolution) * (resolution + 1);

			i += 12;
		}
	}


	// setup vertex buffer and index buffer
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_VertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_IndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	delete[] indices;
}

void TerrainMesh::CreateHeightmapTexture(ID3D11Device* device)
{
	// create heightmap texture

	HRESULT hr;

	// can be local, as reference to texture is kept through SRV/UAV
	ID3D11Texture2D* tex;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = m_HeightmapResolution;
	textureDesc.Height = m_HeightmapResolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS; // texture needs to be accessed by both SRV and UAV
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	hr = device->CreateTexture2D(&textureDesc, nullptr, &tex);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descUAV.Texture2D.MipSlice = 0;
	hr = device->CreateUnorderedAccessView(tex, &descUAV, &m_UAV);

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	descSRV.Format = textureDesc.Format;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(tex, &descSRV, &m_SRV);
}

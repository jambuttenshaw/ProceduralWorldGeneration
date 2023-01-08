#include "TerrainMesh.h"


TerrainMesh::TerrainMesh(ID3D11Device* device)
	: TerrainMesh(device, 256, 100.0f)
{
}

TerrainMesh::TerrainMesh(ID3D11Device* device, unsigned int resolution, float size)
{
	BuildMesh(device, resolution, size);
}

TerrainMesh::~TerrainMesh()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_IndexBuffer) m_IndexBuffer->Release();
}

void TerrainMesh::SendData(ID3D11DeviceContext* deviceContext)
{
	// Set vertex buffer stride and offset.
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void TerrainMesh::BuildMesh(ID3D11Device* device, unsigned int resolution, float size)
{
	HRESULT hr;

	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_IndexBuffer) m_IndexBuffer->Release();

	m_Resolution = resolution;
	m_Size = size;

	m_VertexCount = (resolution + 1) * (resolution + 1);
	m_IndexCount = 6 * resolution * resolution;

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
			indices[i + 0] = (z + 0) + (x + 0) * (m_Resolution + 1);
			indices[i + 1] = (z + 1) + (x + 0) * (m_Resolution + 1);
			indices[i + 2] = (z + 1) + (x + 1) * (m_Resolution + 1);

			indices[i + 3] = (z + 0) + (x + 0) * (m_Resolution + 1);
			indices[i + 4] = (z + 1) + (x + 1) * (m_Resolution + 1);
			indices[i + 5] = (z + 0) + (x + 1) * (m_Resolution + 1);

			i += 6;
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
	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_VertexBuffer);
	assert(hr == S_OK);

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
	assert(hr == S_OK);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	delete[] indices;
}

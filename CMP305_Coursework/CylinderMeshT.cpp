#include "CylinderMeshT.h"
#include <vector>
// Store shape resolution (default is 20), initialise buffers and load texture.
CylinderMeshT::CylinderMeshT(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int stacks, int slices, float height, float bottomRadius, float topRadius)
{
	init(device, stacks, slices, height, bottomRadius, topRadius);
}

// Release resources.
CylinderMeshT::~CylinderMeshT()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}
void CylinderMeshT::initBuffers(ID3D11Device* device)
{
}
// Generate cylinder.
// Shape has texture coordinates and normals.
void CylinderMeshT::init(ID3D11Device* device, int stacks, int slices, float height, float bottomRadius, float topRadius)
{
	std::vector<VertexType> vertices;
	std::vector<UINT> indices;
	
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	float stackHeight = height / stacks;
	UINT ringCount = stacks + 1;
	int sliceCount = 6;

	for (UINT i = 0; i < ringCount; i++) {
		float y = i * stackHeight;
		float r = bottomRadius + ((float)i / stacks)*(topRadius - bottomRadius);

		float dTheta = 2.0f * XM_PI / sliceCount;
		for (UINT j = 0; j <= sliceCount; j++) {
			VertexType vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.position = XMFLOAT3(r * c, y, r * s);
			vertex.texture.x = (float)j / sliceCount;
			vertex.texture.y = 1.0f - (float)i / stacks;
			vertex.normal = XMFLOAT3(r * c, 0.0f, r * s);

			vertices.push_back(vertex);
		}
	}
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < ringCount-1; i++) {
		for (UINT j = 0; j < sliceCount; j++) {
			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j+1);
			indices.push_back((i+1) * ringVertexCount + j);

			indices.push_back(i * ringVertexCount + j);
			indices.push_back(i * ringVertexCount + j+1);
			indices.push_back((i + 1) * ringVertexCount + j+1);
		}
	}



	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	vertexCount = vertices.size();
	indexCount = indices.size();
	vertices.clear();	
	indices.clear();
}


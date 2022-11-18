#include "QuadMeshT.h"
#include <vector>
QuadMeshT::QuadMeshT(ID3D11Device* device, float halfWidth, float halfDepth): 
	hWidth{halfWidth},
	hDepth{ halfDepth }
{
	initBuffers(device);
}

QuadMeshT::~QuadMeshT()
{
}

void QuadMeshT::initBuffers(ID3D11Device* device)
{
	std::vector<VertexType> vertices;
	std::vector <UINT> indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	VertexType v;

	indexCount = 6;

	// Load the vertex array with data.
	v.position = XMFLOAT3(-hWidth, 0.0f, -hDepth);  // Bottom left.
	v.texture = XMFLOAT2(0.0f, 1.0f);
	v.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices.push_back(v);

	v.position = XMFLOAT3(-hWidth, 0.0f, hDepth);  // Top left.
	v.texture = XMFLOAT2(0.0f, 0.0f);
	vertices.push_back(v);

	v.position = XMFLOAT3(hWidth, 0.0f, hDepth);  // Top right.
	v.texture = XMFLOAT2(1.0f, 0.0f);
	vertices.push_back(v);

	v.position = XMFLOAT3(hWidth, 0.0f, -hDepth);  // Bottom right.
	v.texture = XMFLOAT2(1.0f, 1.0f);
	vertices.push_back(v);

	// Load the index array with data.
	indices.push_back(0);  // Bottom left.
	indices.push_back(2);  // Top right.
	indices.push_back(1);  // Top left.

	indices.push_back(0);	// bottom left
	indices.push_back(3);	// bottom right
	indices.push_back(2);	// top right

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = &vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = &indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);	
}

#include "InstancedCubeMesh.h"
#include <vector>

InstancedCubeMesh::InstancedCubeMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT lresolution, UINT instanceCount )
	: CubeMesh( device, deviceContext, lresolution ),
	m_instanceCount(0),
	m_instanceBuffer(0)	
{

}

InstancedCubeMesh::~InstancedCubeMesh() {
	// Run parent destructor
	CubeMesh::~CubeMesh();
}

int InstancedCubeMesh::GetInstanceCount() {
	return m_instanceCount;
}

// Initialise instance buffers.
// Generate and store cube vertices, normals and texture coordinates
void InstancedCubeMesh::initBuffers( ID3D11Device* device, XMFLOAT3* p, XMFLOAT2* uv, UINT count ) {
	
	std::vector<InstanceType> instances;
	D3D11_BUFFER_DESC		instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA	instanceData;
	InstanceType			it;

	// Set the number of instances in the array.
	m_instanceCount = count;

	// Load the instance array with data.
	for(UINT inst = 0; inst < m_instanceCount; inst++ ) {
		it.position = p[inst];
		it.uv = uv[inst];
		instances.push_back(it);
	}

	// Set up the description of the instance buffer.
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof( InstanceType ) * m_instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the instance data.
	instanceData.pSysMem = instances.data();
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// Create the instance buffer.
	device->CreateBuffer( &instanceBufferDesc, &instanceData, m_instanceBuffer.ReleaseAndGetAddressOf() );
}

// Sends geometry data to the GPU. Default primitive topology is TriangleList.
// This function is called instead of sendData in order to send the instance buffer as well as the vertex & index buffers
void InstancedCubeMesh::sendDataInstanced( ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top ) {
	UINT strides[2];
	UINT offsets[2];

	ID3D11Buffer* bufferPointers[2];

	bufferPointers[0] = vertexBuffer;
	bufferPointers[1] = m_instanceBuffer.Get();

	// Set vertex buffer stride and offset.
	strides[0] = sizeof( VertexType );
	strides[1] = sizeof( InstanceType );

	// Set the buffer offsets.
	offsets[0] = 0;
	offsets[1] = 0;

	deviceContext->IASetVertexBuffers( 0, 2, bufferPointers, strides, offsets );
	deviceContext->IASetIndexBuffer( indexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	deviceContext->IASetPrimitiveTopology( top );
}
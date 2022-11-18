/**
* \class Instanced Cube Mesh
*
* \brief Handles setting up the instance buffer for drawing the basic Cube mesh many, many times
*
* Inherits from Cube Mesh, Builds a simple cube with texture coordinates and normals, and sets up the instance buffer.
*
* \author Gaz Robinson
*/
#pragma once

#include "CubeMesh.h"
#include <wrl/client.h>

class InstancedCubeMesh : public CubeMesh {

	//Struct that represents the information that is passed through per-instance
	//Currently only position and UV, but scale and rotation could also be passed through
	struct InstanceType {
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

public:
	InstancedCubeMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT lresolution, UINT instanceCount = 100 );
	~InstancedCubeMesh();

	int GetInstanceCount();

	void initBuffers( ID3D11Device* device, XMFLOAT3* p, XMFLOAT2* uv, UINT count);
	void sendDataInstanced( ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

protected:
	Microsoft::WRL::ComPtr <ID3D11Buffer>	m_instanceBuffer;
	UINT				m_instanceCount;
};


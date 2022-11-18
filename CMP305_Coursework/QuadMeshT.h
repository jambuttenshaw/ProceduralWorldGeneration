#pragma once
#include "BaseMesh.h"
using namespace DirectX;
class QuadMeshT :
    public BaseMesh
{

public:
	QuadMeshT(ID3D11Device* device, float halfWidth, float halfDepth);
	~QuadMeshT();
	XMMATRIX m_Transform;

protected:
	void initBuffers(ID3D11Device* device);
	const float hWidth, hDepth;
};


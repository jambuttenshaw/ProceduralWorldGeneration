#pragma once
#include "PlaneMesh.h"
class TerrainMesh :
	public PlaneMesh {
public:
	TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128 );
	~TerrainMesh();

	void Resize( int newResolution );
	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext );

	inline const int GetResolution() const { return resolution; }
private:
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );
	void BuildHeightMap();

	const float m_UVscale = 10.0f;			//Tile the UV map 10 times across the plane
	const float terrainSize = 100.0f;		//What is the width and height of our terrain
	float* heightMap;
};

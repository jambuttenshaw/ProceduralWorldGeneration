#pragma once

#include <d3d11.h>
#include <DirectXMath.h>


class TerrainMesh
{
	struct VertexType
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 UV;
		DirectX::XMFLOAT3 Normal;
	};

public:
	TerrainMesh(ID3D11Device* device);
	TerrainMesh(ID3D11Device* device, unsigned int resolution, float size);
	~TerrainMesh();

	void SendData(ID3D11DeviceContext* deviceContext);

	void BuildMesh(ID3D11Device* device, unsigned int resolution, float size);

	inline unsigned long GetVertexCount() const { return m_VertexCount; }
	inline unsigned long GetIndexCount() const { return m_IndexCount; }

	inline float GetSize() const { return m_Size; }

private:
	unsigned int m_Resolution = 10; // number of cells along one axis of the terrain mesh
	float m_Size = 100.0f;			// length in units of one edge of the terrain mesh

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;
	unsigned long m_VertexCount = 0, m_IndexCount = 0;
};

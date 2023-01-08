#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;


class Heightmap
{
public:
	Heightmap(ID3D11Device* device, unsigned int resolution);
	~Heightmap();

	ID3D11UnorderedAccessView* GetUAV() const { return m_UAV; }
	ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
	unsigned int GetResolution() const { return m_Resolution; }

	inline const XMFLOAT2& GetOffset() const { return m_Offset; }
	inline void SetOffset(const XMFLOAT2& o) { m_Offset = o; }

private:
	unsigned int m_Resolution = 1024;

	ID3D11ShaderResourceView* m_SRV = nullptr;
	ID3D11UnorderedAccessView* m_UAV = nullptr;

	XMFLOAT2 m_Offset = { 0.0f, 0.0f };
};
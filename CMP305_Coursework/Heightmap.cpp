#include "Heightmap.h"

#include <cassert>

Heightmap::Heightmap(ID3D11Device* device, unsigned int resolution)
	: m_Resolution(resolution)
{
	// create heightmap texture

	HRESULT hr;

	// can be local, as reference to texture is kept through SRV/UAV
	ID3D11Texture2D* tex = nullptr;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = m_Resolution;
	textureDesc.Height = m_Resolution;
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
	assert(hr == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descUAV.Texture2D.MipSlice = 0;
	hr = device->CreateUnorderedAccessView(tex, &descUAV, &m_UAV);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
	descSRV.Format = textureDesc.Format;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MostDetailedMip = 0;
	descSRV.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(tex, &descSRV, &m_SRV);
	assert(hr == S_OK);
}

Heightmap::~Heightmap()
{
	if (m_UAV) m_UAV->Release();
	if (m_SRV) m_SRV->Release();
}

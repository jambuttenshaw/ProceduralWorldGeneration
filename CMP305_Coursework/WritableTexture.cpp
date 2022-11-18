#include "WritableTexture.h"
#include <d3d11.h>

WritableTexture::WritableTexture(int width, int height, ID3D11Device* device):
	m_Width{ width },
	m_Height{ height }
{
	m_PPixels = new UCHAR_Color[m_Width * m_Height];
	for (int y = 0; y < m_Height; y++) {
		for (int x = 0; x < m_Width; x++)
			m_PPixels[x + y * m_Width] = { 0, 0, 0, 255 };
	}

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = m_PPixels;
	initData.SysMemPitch = m_Width * 4;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = m_Width;
	desc.Height = m_Height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&desc, &initData, m_Texture.ReleaseAndGetAddressOf());

	if (SUCCEEDED(hr))
	{
		hr = device->CreateShaderResourceView(m_Texture.Get(), NULL, m_TexView.ReleaseAndGetAddressOf());
	}
}

WritableTexture::~WritableTexture()
{
	delete[] m_PPixels;
	m_PPixels = nullptr;
}

void WritableTexture::SetPixel(int x, int y, float r, float g, float b, float a)
{
	if (x < 0 || x > m_Width - 1 || y < 0 || y > m_Height - 1)
		return;
	m_PPixels[x + y * m_Width] = UCHAR_Color{
		static_cast<unsigned char>(floor(r*255.0)),
		static_cast<unsigned char>(floor(g*255.0)),
		static_cast<unsigned char>(floor(b*255.0)),
		static_cast<unsigned char>(floor(a*255.0)) 
	};
}

void WritableTexture::SetPixel(int x, int y, DirectX::XMFLOAT4 color)
{
	SetPixel(x, y, color.x, color.y, color.z, color.w);
}

void WritableTexture::Update(ID3D11DeviceContext* context) const
{
	context->UpdateSubresource(m_Texture.Get(), 0, NULL, m_PPixels, m_Width * 4, m_Width * m_Height * 4);
}

#include "RenderTarget.h"

RenderTarget::RenderTarget(ID3D11Device* device, unsigned int width, unsigned int height)
	: m_Width(width), m_Height(height)
{
	HRESULT result;

	// create colour buffer
	D3D11_TEXTURE2D_DESC colourBufferDesc;
	ZeroMemory(&colourBufferDesc, sizeof(colourBufferDesc));

	colourBufferDesc.Width = m_Width;
	colourBufferDesc.Height = m_Height;
	colourBufferDesc.MipLevels = 1;
	colourBufferDesc.ArraySize = 1;
	colourBufferDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	colourBufferDesc.SampleDesc.Count = 1;
	colourBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	colourBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	colourBufferDesc.CPUAccessFlags = 0;
	colourBufferDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&colourBufferDesc, NULL, &m_ColourBuffer);

	// create render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));

	renderTargetViewDesc.Format = colourBufferDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_ColourBuffer, &renderTargetViewDesc, &m_RenderTargetView);

	// create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC colourSRVDesc;
	ZeroMemory(&colourSRVDesc, sizeof(colourSRVDesc));

	colourSRVDesc.Format = colourBufferDesc.Format;
	colourSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	colourSRVDesc.Texture2D.MostDetailedMip = 0;
	colourSRVDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_ColourBuffer, &colourSRVDesc, &m_ColourSRV);



	// create depth/stencil buffer
	// Set up the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = m_Width;
	depthBufferDesc.Height = m_Height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	
	result = device->CreateTexture2D(&depthBufferDesc, NULL, &m_DepthStencilBuffer);

	// create depth/stencil vieww
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(m_DepthStencilBuffer, &depthStencilViewDesc, &m_DepthStencilView);

	// create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	ZeroMemory(&depthSRVDesc, sizeof(depthSRVDesc));

	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;
	depthSRVDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_DepthStencilBuffer, &depthSRVDesc, &m_DepthSRV);
}

RenderTarget::~RenderTarget()
{
	if (m_ColourBuffer) m_ColourBuffer->Release();
	if (m_DepthStencilBuffer) m_DepthStencilBuffer->Release();
	if (m_RenderTargetView) m_RenderTargetView->Release();
	if (m_DepthStencilView) m_DepthStencilView->Release();
	if (m_ColourSRV) m_ColourSRV->Release();
	if (m_DepthSRV) m_DepthSRV->Release();
}

void RenderTarget::Clear(ID3D11DeviceContext* deviceContext, const DirectX::XMFLOAT4& colour)
{
	deviceContext->ClearRenderTargetView(m_RenderTargetView, &colour.x);
	deviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderTarget::Set(ID3D11DeviceContext* deviceContext)
{
	deviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);
}

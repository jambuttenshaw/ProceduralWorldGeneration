#include "BaseFullScreenShader.h"

#include <cassert>


BaseFullScreenShader::BaseFullScreenShader(ID3D11Device* device)
	: m_Device(device)
{
}

BaseFullScreenShader::~BaseFullScreenShader()
{
	if (m_VertexShader) m_VertexShader->Release();
	if (m_PixelShader) m_PixelShader->Release();
}

void BaseFullScreenShader::Init(const wchar_t* ps_filename)
{
	LoadVertexShader();
	LoadPixelShader(ps_filename);

	CreateShaderResources();
}

void BaseFullScreenShader::Render(ID3D11DeviceContext* deviceContext)
{
	// Shader resources used by this shader should already be bound prior to calling render

	// no input layout or buffers are used
	// geometry is generated in the vertex shader
	deviceContext->IASetInputLayout(nullptr);
	deviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_PixelShader, nullptr, 0);

	deviceContext->Draw(4, 0);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	UnbindShaderResources(deviceContext);
}

void BaseFullScreenShader::LoadVertexShader(const wchar_t* vs_filename)
{
	HRESULT result;
	ID3DBlob* vertexShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	result = D3DReadFileToBlob(vs_filename, &vertexShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the vertex shader from the buffer.
	result = m_Device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_VertexShader);
	vertexShaderBuffer->Release();
}

void BaseFullScreenShader::LoadPixelShader(const wchar_t* ps_filename)
{
	HRESULT result;
	ID3DBlob* pixelShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	result = D3DReadFileToBlob(ps_filename, &pixelShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the vertex shader from the buffer.
	result = m_Device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_PixelShader);
	pixelShaderBuffer->Release();
}

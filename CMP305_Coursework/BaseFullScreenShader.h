#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>


class BaseFullScreenShader
{
public:
	BaseFullScreenShader(ID3D11Device* device);
	virtual ~BaseFullScreenShader();


	void Render(ID3D11DeviceContext* deviceContext);

protected:
	void Init(const wchar_t* ps_filename);
	virtual void CreateShaderResources() = 0;

	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) = 0;

private:
	void LoadVertexShader(const wchar_t* vs_filename = L"fullscreen_vs.cso");
	void LoadPixelShader(const wchar_t* ps_filename);

protected:
	ID3D11Device* m_Device;

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
};
#pragma once

#include "DXF.h"
#include <wrl/client.h>

using namespace std;
using namespace DirectX;

class LightShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

public:
	LightShader(ID3D11Device* device, HWND hwnd);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, Light* light);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> matrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampleState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lightBuffer;
};


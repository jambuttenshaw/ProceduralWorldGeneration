#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"

#include <nlohmann/json.hpp>

using namespace DirectX;

class WaterShader : public BaseFullScreenShader
{
private:
	struct CameraBufferType
	{
		XMMATRIX projection;
		XMMATRIX invView;
	};

	struct WaterBufferType
	{
		XMMATRIX projection;
		XMFLOAT4 deepColour;
		XMFLOAT4 shallowColour;
		XMFLOAT3 cameraPos;
		float depthMultiplier;
		XMFLOAT3 oceanBoundsMin;
		float alphaMultiplier;
		XMFLOAT3 oceanBoundsMax;
		float normalMapScale;
		float normalMapStrength;
		float smoothness;
		float time;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT4 ambient;
		XMFLOAT4 specular;
		XMFLOAT3 direction;
		float padding;
	};

public:
	WaterShader(ID3D11Device* device, ID3D11ShaderResourceView* normalMapA, ID3D11ShaderResourceView* normalMapB);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth, Light* light, Camera* camera, float time);

	void SettingsGUI();

	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

private:
	ID3D11Buffer* m_CameraBuffer = nullptr;
	ID3D11Buffer* m_WaterBuffer = nullptr;
	ID3D11Buffer* m_LightBuffer = nullptr;
	ID3D11SamplerState* m_NormalMapSamplerState = nullptr;

	XMFLOAT3 m_OceanBoundsMin = { -50.0f, -10.0f, -50.0f };
	XMFLOAT3 m_OceanBoundsMax = { 50.0f, 0.0f, 50.0f };

	XMFLOAT4 m_ShallowColour = { 0.38f, 1.0f, 0.87f, 1.0f };
	XMFLOAT4 m_DeepColour = { 0.10f, 0.22f, 0.6f, 1.0f };

	float m_DepthMultiplier = 0.2f;
	float m_AlphaMultiplier = 0.2f;

	float m_NormalMapStrength = 1.0f;
	float m_NormalMapScale = 20.0f;
	float m_Smoothness = 0.95f;

	ID3D11ShaderResourceView* m_NormalMapA = nullptr;
	ID3D11ShaderResourceView* m_NormalMapB = nullptr;
};

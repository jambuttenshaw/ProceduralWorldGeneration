#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"

#include <nlohmann/json.hpp>

using namespace DirectX;

class BiomeGenerator;


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

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth, Light* light, Camera* camera, float time, const BiomeGenerator* biomeGenerator, const XMINT2& worldPos, int viewSize, float tileSize);

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

	float m_DepthMultiplier = 0.2f;
	float m_AlphaMultiplier = 0.2f;

	float m_NormalMapStrength = 1.0f;
	float m_NormalMapScale = 20.0f;
	float m_Smoothness = 0.95f;

	ID3D11ShaderResourceView* m_NormalMapA = nullptr;
	ID3D11ShaderResourceView* m_NormalMapB = nullptr;
};

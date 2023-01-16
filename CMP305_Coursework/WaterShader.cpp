#include "WaterShader.h"

#include "SerializationHelper.h"

#include "BiomeGenerator.h"


WaterShader::WaterShader(ID3D11Device* device, ID3D11ShaderResourceView* normalMapA, ID3D11ShaderResourceView* normalMapB)
	: BaseFullScreenShader(device),
	m_NormalMapA(normalMapA), m_NormalMapB(normalMapB)
{
	Init(L"water_ps.cso");
}


WaterShader::~WaterShader()
{
	if (m_CameraBuffer) m_CameraBuffer->Release();
	if (m_WaterBuffer) m_WaterBuffer->Release();
	if (m_LightBuffer) m_LightBuffer->Release();
	if (m_NormalMapSamplerState) m_NormalMapSamplerState->Release();
}

void WaterShader::CreateShaderResources()
{
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	m_Device->CreateBuffer(&cameraBufferDesc, NULL, &m_CameraBuffer);

	D3D11_BUFFER_DESC waterBufferDesc;
	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;
	m_Device->CreateBuffer(&waterBufferDesc, NULL, &m_WaterBuffer);

	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	m_Device->CreateBuffer(&lightBufferDesc, NULL, &m_LightBuffer);

	D3D11_SAMPLER_DESC normalMapSamplerDesc;
	normalMapSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	normalMapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	normalMapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	normalMapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	normalMapSamplerDesc.MipLODBias = 0.0f;
	normalMapSamplerDesc.MaxAnisotropy = 1;
	normalMapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	normalMapSamplerDesc.MinLOD = 0;
	normalMapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&normalMapSamplerDesc, &m_NormalMapSamplerState);
}

void WaterShader::UnbindShaderResources(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* nullBuffers[] = { nullptr, nullptr };
	deviceContext->PSSetConstantBuffers(0, 2, nullBuffers);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	deviceContext->PSSetShaderResources(0, 1, &nullSRV);
	ID3D11SamplerState* nullSampler = nullptr;
	deviceContext->PSSetSamplers(0, 1, &nullSampler);
}

void WaterShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth, Light* light, Camera* camera, float time, const BiomeGenerator* biomeGenerator)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	XMMATRIX tinvView = XMMatrixTranspose(XMMatrixInverse(nullptr, viewMatrix));
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	result = deviceContext->Map(m_CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CameraBufferType* cameraPtr;
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->invView = tinvView;
	cameraPtr->projection = tproj;
	deviceContext->Unmap(m_CameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_CameraBuffer);

	result = deviceContext->Map(m_WaterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WaterBufferType* dataPtr;
	dataPtr = (WaterBufferType*)mappedResource.pData;
	
	dataPtr->projection = tproj;
	dataPtr->cameraPos = camera->getPosition();

	dataPtr->oceanBoundsMin = m_OceanBoundsMin;
	dataPtr->oceanBoundsMax = m_OceanBoundsMax;

	dataPtr->depthMultiplier = m_DepthMultiplier;
	dataPtr->alphaMultiplier = m_AlphaMultiplier;
	
	dataPtr->normalMapScale = m_NormalMapScale;
	dataPtr->normalMapStrength = m_NormalMapStrength;
	dataPtr->smoothness = m_Smoothness;

	dataPtr->time = time;
	dataPtr->padding = 0.0f;
	
	deviceContext->Unmap(m_WaterBuffer, 0);

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(m_LightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->specular = light->getSpecularColour();
	lightPtr->direction = light->getDirection();
	lightPtr->padding = 0.0f;
	deviceContext->Unmap(m_LightBuffer, 0);

	ID3D11Buffer* cbs[] = { m_WaterBuffer, m_LightBuffer, biomeGenerator->GetBiomeMappingBuffer() };
	deviceContext->PSSetConstantBuffers(0, 3, cbs);

	ID3D11ShaderResourceView* srvs[] = {
		renderTextureColour, renderTextureDepth,
		m_NormalMapA, m_NormalMapB,
		biomeGenerator->GetBiomeMapSRV(),
		biomeGenerator->GetBiomeTanningSRV()
	};
	deviceContext->PSSetShaderResources(0, 6, srvs);

	deviceContext->PSSetSamplers(0, 1, &m_NormalMapSamplerState);
}

void WaterShader::SettingsGUI()
{
	ImGui::DragFloat3("Ocean Bounds Min", &m_OceanBoundsMin.x, 0.1f);
	ImGui::DragFloat3("Ocean Bounds Max", &m_OceanBoundsMax.x, 0.1f);
	ImGui::SliderFloat("Depth Multiplier", &m_DepthMultiplier, 0.05f, 1.0f);
	ImGui::SliderFloat("Alpha Multiplier", &m_AlphaMultiplier, 0.05f, 1.0f);
	ImGui::SliderFloat("Normal Map Strength", &m_NormalMapStrength, 0.0f, 1.0f);
	ImGui::DragFloat("Normal Map Scale", &m_NormalMapScale, 0.1f);
	ImGui::SliderFloat("Smoothness", &m_Smoothness, 0.0f, 1.0f);
}

nlohmann::json WaterShader::Serialize() const
{
	nlohmann::json serialized;

	serialized["oceanBoundsMin"] = SerializationHelper::SerializeFloat3(m_OceanBoundsMin);
	serialized["oceanBoundsMax"] = SerializationHelper::SerializeFloat3(m_OceanBoundsMax);

	serialized["depthMultiplier"] = m_DepthMultiplier;
	serialized["alphaMultiplier"] = m_AlphaMultiplier;

	serialized["normalMapStrength"] = m_NormalMapStrength;
	serialized["normalMapScale"] = m_NormalMapScale;
	serialized["smoothness"] = m_Smoothness;

	return serialized;
}

void WaterShader::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("oceanBoundsMin")) SerializationHelper::LoadFloat3FromJson(&m_OceanBoundsMin, data["oceanBoundsMin"]);
	if (data.contains("oceanBoundsMax")) SerializationHelper::LoadFloat3FromJson(&m_OceanBoundsMax, data["oceanBoundsMax"]);

	if (data.contains("depthMultiplier")) m_DepthMultiplier = data["depthMultiplier"];
	if (data.contains("alphaMultiplier")) m_AlphaMultiplier = data["alphaMultiplier"];

	if (data.contains("normalMapStrength")) m_NormalMapStrength = data["normalMapStrength"];
	if (data.contains("normalMapScale")) m_NormalMapScale = data["normalMapScale"];
	if (data.contains("smoothness")) m_Smoothness = data["smoothness"];
}

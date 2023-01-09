#include "HeightmapFilters.h"

#include "SerializationHelper.h"
#include "imGUI/imgui.h"
#include <random>


bool SimpleNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Basic Settings"))
	{
		changed |= ImGui::DragFloat("Elevation", &Elevation, 0.01f);
		changed |= ImGui::DragFloat("Frequency", &Frequency, 0.01f);
		changed |= ImGui::DragFloat("Vertical Shift", &VerticalShift, 0.01f);
		changed |= ImGui::DragFloat2("Offset", &Offset.x, 0.001f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Fractal Settings"))
	{
		changed |= ImGui::SliderInt("Octaves", &Octaves, 1, 10);
		changed |= ImGui::DragFloat("Persistence", &Persistence, 0.001f);
		changed |= ImGui::DragFloat("Lacunarity", &Lacunarity, 0.01f);

		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json SimpleNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["elevation"] = Elevation;
	serialized["frequency"] = Frequency;
	serialized["verticalShift"] = VerticalShift;
	serialized["offsetX"] = Offset.x;
	serialized["offsetY"] = Offset.y;

	serialized["octaves"] = Octaves;
	serialized["persistence"] = Persistence;
	serialized["lacunarity"] = Lacunarity;

	return serialized;
}

void SimpleNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("elevation")) Elevation = data["elevation"];
	if (data.contains("frequency")) Frequency = data["frequency"];
	if (data.contains("verticalShift")) VerticalShift = data["verticalShift"];
	if (data.contains("offsetX")) Offset.x = data["offsetX"];
	if (data.contains("offsetY")) Offset.y = data["offsetY"];

	if (data.contains("octaves")) Octaves = data["octaves"];
	if (data.contains("persistence")) Persistence = data["persistence"];
	if (data.contains("lacunarity")) Lacunarity = data["lacunarity"];
}




bool RidgeNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Basic Settings"))
	{
		changed |= ImGui::DragFloat("Elevation", &Elevation, 0.01f);
		changed |= ImGui::DragFloat("Frequency", &Frequency, 0.01f);
		changed |= ImGui::DragFloat("Vertical Shift", &VerticalShift, 0.01f);
		changed |= ImGui::DragFloat2("Offset", &Offset.x, 0.001f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Fractal Settings"))
	{
		changed |= ImGui::SliderInt("Octaves", &Octaves, 1, 10);
		changed |= ImGui::DragFloat("Persistence", &Persistence, 0.001f);
		changed |= ImGui::DragFloat("Lacunarity", &Lacunarity, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Ridge Settings"))
	{
		changed |= ImGui::DragFloat("Power", &Power, 0.01f);
		changed |= ImGui::DragFloat("Gain", &Gain, 0.01f);
		changed |= ImGui::SliderFloat("Peak Smoothing", &PeakSmoothing, 0.0f, 0.5f);

		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json RidgeNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["elevation"] = Elevation;
	serialized["frequency"] = Frequency;
	serialized["verticalShift"] = VerticalShift;
	serialized["offsetX"] = Offset.x;
	serialized["offsetY"] = Offset.y;

	serialized["octaves"] = Octaves;
	serialized["persistence"] = Persistence;
	serialized["lacunarity"] = Lacunarity;

	serialized["power"] = Power;
	serialized["gain"] = Gain;
	serialized["peakSmoothing"] = PeakSmoothing;

	return serialized;
}

void RidgeNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("elevation")) Elevation = data["elevation"];
	if (data.contains("frequency")) Frequency = data["frequency"];
	if (data.contains("verticalShift")) VerticalShift = data["verticalShift"];
	if (data.contains("offsetX")) Offset.x = data["offsetX"];
	if (data.contains("offsetY")) Offset.y = data["offsetY"];

	if (data.contains("octaves")) Octaves = data["octaves"];
	if (data.contains("persistence")) Persistence = data["persistence"];
	if (data.contains("lacunarity")) Lacunarity = data["lacunarity"];

	if (data.contains("power")) Power = data["power"];
	if (data.contains("gain")) Gain = data["gain"];
	if (data.contains("peakSmoothing")) PeakSmoothing = data["peakSmoothing"];
}




bool WarpedSimpleNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Noise Settings"))
	{
		changed |= NoiseSettings.SettingsGUI();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Warp Settings"))
	{
		changed |= WarpSettings.SettingsGUI();
		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json WarpedSimpleNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["noiseSettings"] = NoiseSettings.Serialize();
	serialized["warpSettings"] = WarpSettings.Serialize();

	return serialized;
}

void WarpedSimpleNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("noiseSettings")) NoiseSettings.LoadFromJson(data["noiseSettings"]);
	if (data.contains("warpSettings")) WarpSettings.LoadFromJson(data["warpSettings"]);
}




bool TerrainNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Continent Settings"))
	{
		changed |= ContinentSettings.SettingsGUI();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Mountain Settings"))
	{
		changed |= MountainSettings.SettingsGUI();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Ocean Settings"))
	{
		changed |= ImGui::DragFloat("Ocean Depth Multiplier", &OceanDepthMultiplier, 0.01f);
		changed |= ImGui::DragFloat("Ocean Floor Depth", &OceanFloorDepth, 0.01f);
		changed |= ImGui::SliderFloat("Ocean Floor Smoothing", &OceanFloorSmoothing, 0.0f, 5.0f);
		changed |= ImGui::DragFloat("Mountain Blend", &MountainBlend, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Warp Settings"))
	{
		changed |= WarpSettings.SettingsGUI();
		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json TerrainNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["continentSettings"] = ContinentSettings.Serialize();
	serialized["mountainSettings"] = MountainSettings.Serialize();
	serialized["warpSettings"] = WarpSettings.Serialize();

	serialized["oceanDepthMultiplier"] = OceanDepthMultiplier;
	serialized["oceanFloorDepth"] = OceanFloorDepth;
	serialized["oceanFloorSmoothing"] = OceanFloorSmoothing;
	serialized["mountainBlend"] = MountainBlend;

	return serialized;
}

void TerrainNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("continentSettings")) ContinentSettings.LoadFromJson(data["continentSettings"]);
	if (data.contains("mountainSettings")) MountainSettings.LoadFromJson(data["mountainSettings"]);
	if (data.contains("warpSettings")) WarpSettings.LoadFromJson(data["warpSettings"]);

	if (data.contains("oceanDepthMultiplier")) OceanDepthMultiplier = data["oceanDepthMultiplier"];
	if (data.contains("oceanFloorDepth")) OceanFloorDepth = data["oceanFloorDepth"];
	if (data.contains("oceanFloorSmoothing")) OceanFloorSmoothing = data["oceanFloorSmoothing"];
	if (data.contains("mountainBlend")) MountainBlend = data["mountainBlend"];
}




bool VoronoiBiomeSettings::SettingsGUI()
{
	bool changed = false;

	changed |= ImGui::DragFloatRange2("Point X Range", &MinPointBounds.x, &MaxPointBounds.x, 0.01f);
	changed |= ImGui::DragFloatRange2("Point Y Range", &MinPointBounds.y, &MaxPointBounds.y, 0.01f);
	changed |= ImGui::SliderInt("Num Points", &PointCount, 1, 256);
	changed |= ImGui::InputInt("Biome Seed", &BiomeSeed);
	if (ImGui::InputInt("Num Biome Types", &NumBiomeTypes))
	{
		NumBiomeTypes = max(1, NumBiomeTypes);
		changed = true;
	}
	changed |= ImGui::SliderFloat("Transition Threshold", &TransitionThreshold, 0.0f, 1.0f);
	if (ImGui::TreeNode("Transition Noise Settings"))
	{
		changed |= TransitionNoiseSettings.SettingsGUI();
		ImGui::TreePop();
	}
	return changed;
}

nlohmann::json VoronoiBiomeSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["minPointBounds"] = SerializationHelper::SerializeFloat2(MinPointBounds);
	serialized["maxPointBounds"] = SerializationHelper::SerializeFloat2(MaxPointBounds);
	serialized["numPoints"] = PointCount;
	serialized["biomeSeed"] = BiomeSeed;
	serialized["numBiomeTypes"] = NumBiomeTypes;
	serialized["transitionThreshold"] = TransitionThreshold;
	serialized["transitionNoiseSettings"] = TransitionNoiseSettings.Serialize();

	return serialized;
}
void VoronoiBiomeSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("minPointBounds")) SerializationHelper::LoadFloat2FromJson(&MinPointBounds, data["minPointBounds"]);
	if (data.contains("maxPointBounds")) SerializationHelper::LoadFloat2FromJson(&MaxPointBounds, data["maxPointBounds"]);
	if (data.contains("numPoints")) PointCount = data["numPoints"];
	if (data.contains("biomeSeed")) BiomeSeed = data["biomeSeed"];
	if (data.contains("numBiomeTypes")) NumBiomeTypes = data["numBiomeTypes"];
	if (data.contains("transitionThreshold")) TransitionThreshold = data["transitionThreshold"];
	if (data.contains("transitionNoiseSettings")) TransitionNoiseSettings.LoadFromJson(data["transitionNoiseSettings"]);
}

VoronoiBiomesFilter::VoronoiBiomesFilter(ID3D11Device* device)
	: BaseHeightmapFilter(device, L"voronoiBiomes_cs.cso")
{
	HRESULT hr;

	D3D11_BUFFER_DESC pointsBufferDesc;
	pointsBufferDesc.ByteWidth = m_MaxPoints * sizeof(BiomeType);
	pointsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pointsBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	pointsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pointsBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	pointsBufferDesc.StructureByteStride = sizeof(BiomeType);

	hr = device->CreateBuffer(&pointsBufferDesc, nullptr, &m_PointsBuffer);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_MaxPoints;

	hr = device->CreateShaderResourceView(m_PointsBuffer, &srvDesc, &m_PointsSRV);
	assert(hr == S_OK);
}

VoronoiBiomesFilter::~VoronoiBiomesFilter()
{
	m_PointsBuffer->Release();
	m_PointsSRV->Release();
}


void VoronoiBiomesFilter::Run(ID3D11DeviceContext* deviceContext, Heightmap* heightmap)
{
	HRESULT hr;

	// create voronoi points
	BiomeType* points = new BiomeType[m_Settings.PointCount];

	std::mt19937 rng(m_Settings.BiomeSeed);
	std::uniform_real_distribution<float> xDist(m_Settings.MinPointBounds.x, m_Settings.MaxPointBounds.x);
	std::uniform_real_distribution<float> yDist(m_Settings.MinPointBounds.y, m_Settings.MaxPointBounds.y);
	std::uniform_int_distribution<int> typeDist(1, max(m_Settings.NumBiomeTypes, 1));

	for (int i = 0; i < m_Settings.PointCount; i++)
	{
		points[i] = BiomeType
		{
			{ xDist(rng), yDist(rng) },
			typeDist(rng)
		};
	}

	// load into structured buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = deviceContext->Map(m_PointsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	memcpy(mappedResource.pData, points, m_Settings.PointCount * sizeof(BiomeType));
	deviceContext->Unmap(m_PointsBuffer, 0);

	deviceContext->CSSetShaderResources(0, 1, &m_PointsSRV);

	// run CS
	BaseHeightmapFilter::Run(deviceContext, heightmap);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	deviceContext->CSSetShaderResources(0, 1, &nullSRV);

	delete[] points;
}

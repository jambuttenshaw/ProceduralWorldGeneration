#pragma once

#include "BaseHeightmapFilter.h"
#include "nlohmann/json.hpp"

// SETTINGS DEFINITIONS
// these structs must EXACTLY match the HeightmapSettingsBuffer cbuffer in the filters respective compute shader
// these structs must also be >>>>>>PADDED IN 16 BYTE CHUNKS<<<<<<
// these structs are transferred DIRECTLY into the compute shaders settings cbuffer!!!

struct SimpleNoiseSettings
{
	// 16 bytes
	float Elevation = 1.0f;
	float Frequency = 1.0f;
	float VerticalShift = 0.0f;
	int Octaves = 4;
	// 16 bytes
	DirectX::XMFLOAT2 Offset{ 0.0f, 0.0f };
	float Persistence = 0.5f;
	float Lacunarity = 2.0f;

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};

struct RidgeNoiseSettings
{
	// 16 bytes
	float Elevation = 8.0f;
	float Frequency = 0.5f;
	float VerticalShift = 0.0f;
	int Octaves = 8;
	// 16 bytes
	DirectX::XMFLOAT2 Offset{ 0.0f, 0.0f };
	float Persistence = 0.6f;
	float Lacunarity = 2.2f;
	// 16 bytes
	float Power = 5.0f;
	float Gain = 7.0f;
	float PeakSmoothing = 0.0f;
	float Padding = 0.0f;

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};

struct WarpedSimpleNoiseSettings
{
	SimpleNoiseSettings WarpSettings;
	SimpleNoiseSettings NoiseSettings;

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};


struct TerrainNoiseSettings
{
	SimpleNoiseSettings WarpSettings;
	SimpleNoiseSettings ContinentSettings;
	RidgeNoiseSettings MountainSettings;

	float OceanDepthMultiplier = 0.0f;
	float OceanFloorDepth = 3.0f;
	float OceanFloorSmoothing = 0.5f;
	float MountainBlend = 0.0f;

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};


// FILTER DEFINITIONS

class SimpleNoiseFilter : public BaseHeightmapFilter<SimpleNoiseSettings>
{
public:
	SimpleNoiseFilter(ID3D11Device* device)
		: BaseHeightmapFilter(device, L"simpleNoise_cs.cso") {}
	virtual ~SimpleNoiseFilter() = default;

	inline virtual const char* Label() const override { return "Simple Noise"; }
};


class RidgeNoiseFilter : public BaseHeightmapFilter<RidgeNoiseSettings>
{
public:
	RidgeNoiseFilter(ID3D11Device* device)
		: BaseHeightmapFilter(device, L"ridgeNoise_cs.cso") {}
	virtual ~RidgeNoiseFilter() = default;

	inline virtual const char* Label() const override { return "Ridge Noise"; }
};


class WarpedSimpleNoiseFilter : public BaseHeightmapFilter<WarpedSimpleNoiseSettings>
{
public:
	WarpedSimpleNoiseFilter(ID3D11Device* device)
		: BaseHeightmapFilter(device, L"warpedSimpleNoise_cs.cso") {}
	virtual ~WarpedSimpleNoiseFilter() = default;

	inline virtual const char* Label() const override { return "Warped Simple Noise"; }
};


class TerrainNoiseFilter : public BaseHeightmapFilter<TerrainNoiseSettings>
{
public:
	TerrainNoiseFilter(ID3D11Device* device)
		: BaseHeightmapFilter(device, L"terrainNoise_cs.cso") {}
	virtual ~TerrainNoiseFilter() = default;

	inline virtual const char* Label() const override { return "Terrain Noise"; }
};
#pragma once

#include "nlohmann/json.hpp"
#include <DirectXMath.h>

using namespace DirectX;

// SETTINGS DEFINITIONS
// these structs must EXACTLY match the HeightmapSettingsBuffer cbuffer in the filters respective compute shader
// these structs must also be >>>>>>PADDED IN 16 BYTE CHUNKS<<<<<<
// these structs are transferred DIRECTLY into the compute shaders settings cbuffer!!!

struct SimpleNoiseSettings
{
	float Elevation = 0.0f;
	float Frequency = 1.0f;
	float VerticalShift = 0.0f;
	int Octaves = 4;
	XMFLOAT2 Offset{ 0.0f, 0.0f };
	float Persistence = 0.5f;
	float Lacunarity = 2.0f;

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};

struct MountainNoiseSettings
{
	float Elevation = 20.0f;
	float Frequency = 0.5f;
	int Octaves = 8;
	float Persistence = 0.5f;
	float Lacunarity = 2.1f;
	XMFLOAT2 Offset{ 0.0f, 0.0f };
	float Blending = 0.15f;
	float Detail = 0.1f;

	XMFLOAT3 Padding{ 0.0f, 0.0f, 0.0f };

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};

struct RidgeNoiseSettings
{
	float Elevation = 0.0f;
	float Frequency = 0.5f;
	int Octaves = 8;
	float Persistence = 0.6f;
	float Lacunarity = 2.2f;
	XMFLOAT2 Offset{ 0.0f, 0.0f };
	float Power = 5.0f;
	float Gain = 7.0f;
	float RidgeThreshold = 2.0f;

	XMFLOAT2 Padding{ 0.0f, 0.0f };

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};


struct TerrainNoiseSettings
{
	SimpleNoiseSettings ContinentSettings;
	MountainNoiseSettings MountainSettings;
	RidgeNoiseSettings RidgeSettings;

	float OceanDepthMultiplier = 0.0f;
	float OceanFloorDepth = 3.0f;
	float OceanFloorSmoothing = 0.5f;
	float Padding = 0.0f;

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);
};
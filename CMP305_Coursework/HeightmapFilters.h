#pragma once

#include "BaseHeightmapFilter.h"
#include "nlohmann/json.hpp"
#include "NoiseSettings.h"

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

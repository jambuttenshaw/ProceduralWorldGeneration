#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <random>
#include <map>
#include <functional>

#include "NoiseSettings.h"

using namespace DirectX;

#define MAX_BIOMES 32




class BiomeGenerator 
{
public:

	enum BIOME_TYPE : int
	{
		BIOME_TYPE_OCEAN = 0,
		BIOME_TYPE_LAND = 1
	};
	enum BIOME_TEMP : int
	{
		BIOME_TEMP_COLD = 0,
		BIOME_TEMP_TEMPERATE = 1,
		BIOME_TEMP_WARM = 2
	};

	struct Biome
	{
		// debug
		const char* name = nullptr;

		// spawning properties
		BIOME_TYPE type;
		BIOME_TEMP temperature;
		int spawnWeight;
	};

	struct BiomeTan
	{
		XMFLOAT3 shoreColour{ 0.89f, 0.8f, 0.42f };
		XMFLOAT3 flatColour{ 0.3f, 0.5f, 0.05f };
		XMFLOAT3 flatDetailColour{ 0.3f, 0.5f, 0.05f };
		XMFLOAT3 slopeColour{ 0.35f, 0.23f, 0.04f };
		XMFLOAT3 cliffColour{ 0.19f, 0.18f, 0.15f };
		XMFLOAT3 snowColour{ 1.0f, 1.0f, 1.0f };
		XMFLOAT3 shallowWaterColour{ 0.38f, 1.0f, 0.87f };
		XMFLOAT3 deepWaterColour{ 0.1f, 0.22f, 0.6f };

		float flatThreshold = 0.69f;
		float cliffThreshold = 0.89f;
		float shoreHeight = 1.5f;
		
		float snowHeight = 23.0f;
		float snowSteepness = 0.7f;
		float snowSmoothing = 0.1f;

		float detailThreshold = 0.0f;
		float detailScale = 4.0f;

		float steepnessSmoothing = 0.125f;
		float heightSmoothing = 3.0f;

		bool SettingsGUI();
		nlohmann::json Serialize() const;
		void LoadFromJson(const nlohmann::json& data);
	};

	struct BiomeMappingBufferType
	{
		XMFLOAT2 topleft;
		float scale;
		unsigned int resolution;
		float blending;
		XMFLOAT3 padding;
	};

	BiomeGenerator(ID3D11Device* device, unsigned int seed);
	~BiomeGenerator();

	bool SettingsGUI();
	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);

	void GenerateBiomeMap(ID3D11Device* device);

	inline size_t GetBiomeCount() const { return m_AllBiomes.size(); }

	inline ID3D11ShaderResourceView* GetBiomeMapSRV() const { return m_BiomeMapSRV; }
	inline size_t GetBiomeMapResolution() const { return m_BiomeMapSize; }
	
	inline ID3D11Buffer* GetBiomeMappingBuffer() const { return m_BiomeMappingBuffer; }
	
	inline ID3D11ShaderResourceView* GetGenerationSettingsSRV() const { return m_GenerationSettingsView; }
	inline ID3D11ShaderResourceView* GetBiomeTanningSRV() const { return m_BiomeTanView; }
	inline const XMFLOAT4* GetBiomeMinimapColours() const { return m_BiomeMinimapColours; }

	void UpdateBuffers(ID3D11DeviceContext* deviceContext);

	inline bool ShowBiomeMap() const { return m_ShowBiomeMap; }

private:
	// land/ocean balance
	void IslandsCA(int** biomeMapPtr, size_t mapSize);
	void AddIslandsCA(int** biomeMapPtr, size_t mapSize);
	void RemoveTooMuchOcean(int** biomeMapPtr, size_t mapSize);

	// biome temperatures
	void CreateTemperatures(int** tempMapPtr, int* biomeMap, size_t mapSize);
	void TransitionTemperatures(int** tempMapPtr, size_t mapSize);

	void SelectBiomes(int** biomeMapPtr, int* tempMap, size_t mapSize);
	void AddShores(int** biomeMapPtr, size_t mapSize);

	// zoom
	void Zoom2x(int** mapPtr, size_t* mapSize);

	// utility
	int CountNeighboursEqual(int x, int y, int v, int* biomeMap, size_t mapSize);
	int CountNeighboursEqual(int x, int y, std::function<bool(int biome)> condition, int* biomeMap, size_t mapSize);
	int GetBiomeIDByName(const char* name) const;

	void CreateBiomeMapTexture(ID3D11Device* device);
	void CreateBiomeMappingBuffer(ID3D11Device* device);
	void CreateGenerationSettingsBuffer(ID3D11Device* device);
	void CreateBiomeTanBuffer(ID3D11Device* device);

	inline bool Chance(int percent) { return m_Chance(m_RNG) <= percent; }

	const char* StrFromBiomeType(BIOME_TYPE type);
	const char* StrFromBiomeTemp(BIOME_TEMP temp);

private:
	ID3D11Device* m_Device = nullptr;

	std::mt19937 m_RNG;
	unsigned int m_Seed;
	std::uniform_int_distribution<int> m_Chance;

	std::vector<Biome> m_AllBiomes;
	std::map<BIOME_TEMP, std::vector<int>> m_SpawnableLandBiomesByTemp;
	std::map<BIOME_TEMP, std::vector<int>> m_SpawnableOceanBiomesByTemp;

	// biome generation constants (all are integers [0,100] for how likely something is to occur)
	int m_ContinentChance = 15;
	int m_IslandExpandChance = 30;
	int m_IslandErodeChance = 15;
	int m_RemoveTooMuchOceanChance = 45;
	int m_ZoomSamplePerturbation = 15;

	int m_TemperateBiomeChance = 66;
	int m_WarmBiomeChance = 17;
	int m_ColdBiomeChance = 17;

	int* m_BiomeMap = nullptr;
	size_t m_BiomeMapSize = -1;
	ID3D11ShaderResourceView* m_BiomeMapSRV = nullptr;

	TerrainNoiseSettings m_GenerationSettings[MAX_BIOMES];
	ID3D11Buffer* m_GenerationSettingsBuffer = nullptr;
	ID3D11ShaderResourceView* m_GenerationSettingsView = nullptr;

	BiomeTan m_BiomeTans[MAX_BIOMES];
	ID3D11Buffer* m_BiomeTanBuffer = nullptr;
	ID3D11ShaderResourceView* m_BiomeTanView = nullptr;

	ID3D11Buffer* m_BiomeMappingBuffer = nullptr;
	XMFLOAT2 m_BiomeMapTopLeft{ 0, 0 };
	float m_BiomeMapScale = 8.0f;
	float m_BiomeBlending = 0.5f;

	XMFLOAT4 m_BiomeMinimapColours[MAX_BIOMES];

	// gui
	bool m_ShowBiomeMap = true;
};
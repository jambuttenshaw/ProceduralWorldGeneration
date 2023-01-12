#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <random>
#include <map>

#include "NoiseSettings.h"

using namespace DirectX;

#define MAX_BIOMES 16




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
		bool variant;
		int spawnWeight;

		// generation settings specific to this biome are stored in the Generation Settings buffer
	};

	struct BiomeTan
	{
		XMFLOAT3 shoreColour{ 0.89f, 0.8f, 0.42f };
		XMFLOAT3 flatColour{ 0.3f, 0.5f, 0.05f };
		XMFLOAT3 slopeColour{ 0.35f, 0.23f, 0.04f };
		XMFLOAT3 cliffColour{ 0.19f, 0.18f, 0.15f };

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

	bool GenerationSettingsGUI();
	bool TanningSettingsGUI();
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

private:
	// land/ocean balance
	void IslandsCA(int** biomeMapPtr, size_t mapSize);
	void AddIslandsCA(int** biomeMapPtr, size_t mapSize);
	void RemoveTooMuchOcean(int** biomeMapPtr, size_t mapSize);

	// biome temperatures
	void CreateTemperatures(int** tempMapPtr, int* biomeMap, size_t mapSize);
	void TransitionTemperatures(int** tempMapPtr, size_t mapSize);

	void SelectBiomes(int** biomeMapPtr, int* tempMap, size_t mapSize);

	// zoom
	void Zoom2x(int** biomeMapPtr, size_t* mapSize);

	// utility
	int CountNeighboursEqual(int x, int y, int v, int* biomeMap, size_t mapSize);
	int GetBiomeIDByName(const char* name) const;
	void GetAllBiomesWithTemperature(std::vector<int>& out, BIOME_TEMP temperature);

	void CreateBiomeMapTexture(ID3D11Device* device);
	void CreateGenerationSettingsBuffer(ID3D11Device* device);
	void CreateBiomeTanBuffer(ID3D11Device* device);

	inline bool Chance(int percent) { return m_Chance(m_RNG) <= percent; }

	const char* StrFromBiomeType(BIOME_TYPE type);
	const char* StrFromBiomeTemp(BIOME_TEMP temp);

private:
	std::mt19937 m_RNG;
	std::uniform_int_distribution<int> m_Chance;

	std::vector<Biome> m_AllBiomes;
	std::vector<int> m_ColdBiomes;
	std::vector<int> m_TemperateBiomes;
	std::vector<int> m_WarmBiomes;
	std::map<BIOME_TEMP, std::vector<int>*> m_BiomesByTemp;

	// biome generation constants (all are integers [1,100] for how likely something is to occur)
	int m_ContinentChance = 15;
	int m_IslandExpandChance = 30;
	int m_IslandErodeChance = 15;
	int m_RemoveTooMuchOceanChance = 45;

	int* m_BiomeMap = nullptr;
	size_t m_BiomeMapSize = -1;
	ID3D11ShaderResourceView* m_BiomeMapSRV = nullptr;

	int* m_TemperatureMap = nullptr;
	size_t m_TemperatureMapSize = -1;

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
};
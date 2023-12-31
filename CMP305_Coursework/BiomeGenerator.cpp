#include "BiomeGenerator.h"

#include <algorithm>
#include <cassert>

#include "imGUI/imgui.h"
#include "SerializationHelper.h"





static bool BiomeNameItemGetter(void* data, int idx, const char** out_str)
{
	*out_str = ((BiomeGenerator::Biome*)data + idx)->name;
	return true;
}

bool BiomeGenerator::BiomeTan::SettingsGUI()
{
	bool changed = false;

	changed |= ImGui::ColorEdit3("Shore", &shoreColour.x);
	changed |= ImGui::ColorEdit3("Flat", &flatColour.x);
	changed |= ImGui::ColorEdit3("Flat Detail", &flatDetailColour.x);
	changed |= ImGui::ColorEdit3("Slope", &slopeColour.x);
	changed |= ImGui::ColorEdit3("Cliff", &cliffColour.x);
	changed |= ImGui::ColorEdit3("Snow", &snowColour.x);

	changed |= ImGui::ColorEdit3("Shallow Water", &shallowWaterColour.x);
	changed |= ImGui::ColorEdit3("Deep Water", &deepWaterColour.x);

	changed |= ImGui::SliderFloat("Flat threshold", &flatThreshold, 0.0f, cliffThreshold);
	changed |= ImGui::SliderFloat("Cliff threshold", &cliffThreshold, flatThreshold, 1.0f);
	changed |= ImGui::DragFloat("Shore Height", &shoreHeight, 0.005f);
	changed |= ImGui::SliderFloat("Detail Threshold", &detailThreshold, 0.0f, 1.0f);
	ImGui::Text("Snow");
	changed |= ImGui::DragFloat("Snow Height", &snowHeight, 0.005f);
	changed |= ImGui::SliderFloat("Snow Steepness", &snowSteepness, 0.0f, 1.0f);
	changed |= ImGui::SliderFloat("Snow Smoothing", &snowSmoothing, 0.0f, 1.0f);

	ImGui::Text("Smoothing");
	changed |= ImGui::SliderFloat("Steepness Smoothing", &steepnessSmoothing, 0.0f, 0.5f);
	changed |= ImGui::DragFloat("Height Smoothing", &heightSmoothing, 0.01f);

	return changed;
}

nlohmann::json BiomeGenerator::BiomeTan::Serialize() const
{
	auto serialized = nlohmann::json();

	serialized["shore"] = SerializationHelper::SerializeFloat3(shoreColour);
	serialized["flat"] = SerializationHelper::SerializeFloat3(flatColour);
	serialized["flatDetail"] = SerializationHelper::SerializeFloat3(flatDetailColour);
	serialized["slope"] = SerializationHelper::SerializeFloat3(slopeColour);
	serialized["cliff"] = SerializationHelper::SerializeFloat3(cliffColour);
	serialized["snow"] = SerializationHelper::SerializeFloat3(snowColour);
	serialized["shallowWater"] = SerializationHelper::SerializeFloat3(shallowWaterColour);
	serialized["deepWater"] = SerializationHelper::SerializeFloat3(deepWaterColour);
	
	serialized["flatThreshold"] = flatThreshold;
	serialized["cliffThreshold"] = cliffThreshold;
	serialized["shoreHeight"] = shoreHeight;
	serialized["detailThreshold"] = detailThreshold;
	serialized["snowHeight"] = snowHeight;
	serialized["snowSteepness"] = snowSteepness;
	serialized["snowSmoothing"] = snowSmoothing;
	serialized["steepnessSmoothing"] = steepnessSmoothing;
	serialized["heightSmoothing"] = heightSmoothing;

	return serialized;
}

void BiomeGenerator::BiomeTan::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("shore")) SerializationHelper::LoadFloat3FromJson(&shoreColour, data["shore"]);
	if (data.contains("flat")) SerializationHelper::LoadFloat3FromJson(&flatColour, data["flat"]);
	if (data.contains("flatDetail")) SerializationHelper::LoadFloat3FromJson(&flatDetailColour, data["flatDetail"]);
	if (data.contains("slope")) SerializationHelper::LoadFloat3FromJson(&slopeColour, data["slope"]);
	if (data.contains("cliff")) SerializationHelper::LoadFloat3FromJson(&cliffColour, data["cliff"]);
	if (data.contains("snow")) SerializationHelper::LoadFloat3FromJson(&snowColour, data["snow"]);
	if (data.contains("shallowWater")) SerializationHelper::LoadFloat3FromJson(&shallowWaterColour, data["shallowWater"]);
	if (data.contains("deepWater")) SerializationHelper::LoadFloat3FromJson(&deepWaterColour, data["deepWater"]);

	if (data.contains("flatThreshold")) flatThreshold = data["flatThreshold"];
	if (data.contains("cliffThreshold")) cliffThreshold = data["cliffThreshold"];
	if (data.contains("shoreHeight")) shoreHeight = data["shoreHeight"];
	if (data.contains("detailThreshold")) detailThreshold = data["detailThreshold"];
	if (data.contains("snowHeight")) snowHeight = data["snowHeight"];
	if (data.contains("snowSteepness")) snowSteepness = data["snowSteepness"];
	if (data.contains("snowSmoothing")) snowSmoothing = data["snowSmoothing"];
	if (data.contains("steepnessSmoothing")) steepnessSmoothing = data["steepnessSmoothing"];
	if (data.contains("heightSmoothing")) heightSmoothing = data["heightSmoothing"];
}



BiomeGenerator::BiomeGenerator(ID3D11Device* device, unsigned int seed)
	: m_Device(device), m_RNG(seed), m_Chance(1, 100)
{
	// create biomes
	m_AllBiomes.clear();
	m_AllBiomes.push_back({ "Ocean",			BIOME_TYPE_OCEAN, BIOME_TEMP_TEMPERATE, 10 });
	m_AllBiomes.push_back({ "Plains",			BIOME_TYPE_LAND,  BIOME_TEMP_TEMPERATE, 2 });
	m_AllBiomes.push_back({ "Mountains",		BIOME_TYPE_LAND,  BIOME_TEMP_TEMPERATE, 1 });
	m_AllBiomes.push_back({ "Desert",			BIOME_TYPE_LAND,  BIOME_TEMP_WARM,		2 });
	m_AllBiomes.push_back({ "Desert Hills",		BIOME_TYPE_LAND,  BIOME_TEMP_WARM,		1 });
	m_AllBiomes.push_back({ "Tundra",			BIOME_TYPE_LAND,  BIOME_TEMP_COLD,		2 });
	m_AllBiomes.push_back({ "Snowy Mountains",	BIOME_TYPE_LAND,  BIOME_TEMP_COLD,		1 });
	m_AllBiomes.push_back({ "Shore",			BIOME_TYPE_LAND,  BIOME_TEMP_TEMPERATE,	1 });
	m_AllBiomes.push_back({ "Cold Shore",		BIOME_TYPE_LAND,  BIOME_TEMP_COLD,		1 });
	m_AllBiomes.push_back({ "Islands",			BIOME_TYPE_OCEAN, BIOME_TEMP_TEMPERATE,	1 });
	m_AllBiomes.push_back({ "Warm Ocean",		BIOME_TYPE_OCEAN, BIOME_TEMP_TEMPERATE,	1 });
	m_AllBiomes.push_back({ "Cold Ocean",		BIOME_TYPE_OCEAN, BIOME_TEMP_TEMPERATE,	1 });

	// process biome list
#define BID(x) GetBiomeIDByName(x)

	// set up spawnable biomes
	m_SpawnableLandBiomesByTemp.insert(std::make_pair(BIOME_TEMP_COLD,
		std::vector<int>({ BID("Tundra"), BID("Snowy Mountains") })));

	m_SpawnableLandBiomesByTemp.insert(std::make_pair( BIOME_TEMP_TEMPERATE,
		std::vector<int>({ BID("Plains"), BID("Mountains") })));

	m_SpawnableLandBiomesByTemp.insert(std::make_pair( BIOME_TEMP_WARM,
		std::vector<int>({ BID("Desert"), BID("Desert Hills") })));


	m_SpawnableOceanBiomesByTemp.insert(std::make_pair(BIOME_TEMP_COLD,
		std::vector<int>({ BID("Cold Ocean") })));

	m_SpawnableOceanBiomesByTemp.insert(std::make_pair(BIOME_TEMP_TEMPERATE,
		std::vector<int>({ BID("Ocean"), BID("Islands") })));

	m_SpawnableOceanBiomesByTemp.insert(std::make_pair(BIOME_TEMP_WARM,
		std::vector<int>({ BID("Warm Ocean") })));

	// create resources buffers
	CreateBiomeMappingBuffer(device);
	CreateGenerationSettingsBuffer(device);
	CreateBiomeTanBuffer(device);
}

BiomeGenerator::~BiomeGenerator()
{
	if (m_BiomeMap) delete m_BiomeMap;
	if (m_BiomeMapSRV) m_BiomeMapSRV->Release();

	if (m_GenerationSettingsBuffer) m_GenerationSettingsBuffer->Release();
	if (m_GenerationSettingsView) m_GenerationSettingsView->Release();

	if (m_BiomeTanBuffer) m_BiomeTanBuffer->Release();
	if (m_BiomeTanView) m_BiomeTanView->Release();

	if (m_BiomeMappingBuffer) m_BiomeMappingBuffer->Release();
}

bool BiomeGenerator::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Biome Mapping"))
	{
		ImGui::Separator();

		ImGui::Text("Biome map size: %d", m_BiomeMapSize);
		ImGui::Text("Mapping:");
		changed |= ImGui::DragFloat("Pixels Per Tile", &m_BiomeMapPxPerTile, 0.01f);
		changed |= ImGui::SliderFloat("Blending", &m_BiomeBlending, 0.0f, 1.0f);
	
		ImGui::Checkbox("Show Biome Map", &m_ShowBiomeMap);

		ImGui::Separator();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Biome Map Generation"))
	{
		ImGui::Separator();

		// these values don't modify 'changed' as they don't affect world generation, only biome generation
		ImGui::InputInt("Seed", (int*)(&m_Seed));
		ImGui::Text("Generation Probabilities:");
		ImGui::SliderInt("Continent", &m_ContinentChance, 0, 100);
		ImGui::SliderInt("Island Expand", &m_IslandExpandChance, 0, 100);
		ImGui::SliderInt("Island Erode", &m_IslandErodeChance, 0, 100);
		ImGui::SliderInt("Small Islands", &m_SmallIslandsChance, 0, 100);
		ImGui::SliderInt("Zooming Perturbations", &m_ZoomSamplePerturbation, 0, 100);
		ImGui::DragInt("Cold Biome", &m_ColdBiomeChance, 0.1f);
		ImGui::DragInt("Temperate Biome", &m_TemperateBiomeChance, 0.1f);
		ImGui::DragInt("Warm Biome", &m_WarmBiomeChance, 0.1f);

		ImGui::Separator();

		if (ImGui::Button("Regenerate Biome Map"))
		{
			GenerateBiomeMap(m_Device);
			changed = true;
		}

		ImGui::Separator();
		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Biome Settings"))
	{
		ImGui::Separator();

		static int selectedBiome = 0;
		ImGui::Combo("Biome", &selectedBiome, &BiomeNameItemGetter, m_AllBiomes.data(), static_cast<int>(m_AllBiomes.size()));

		Biome& biome = m_AllBiomes.at(selectedBiome);
		ImGui::Separator();
		ImGui::Text("%s Biome Properties:", biome.name);
		ImGui::Text("Type: %s", StrFromBiomeType(biome.type));
		ImGui::Text("Temperature: %s", StrFromBiomeTemp(biome.temperature));

		ImGui::InputInt("Spawn Weight", &biome.spawnWeight);
		ImGui::ColorEdit3("Biome Map Colour", (float*)(m_BiomeMinimapColours + selectedBiome));

		ImGui::Separator();
		if (ImGui::TreeNode("Heightmap Generation"))
		{
			changed |= m_GenerationSettings[selectedBiome].SettingsGUI();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Tanning"))
		{
			m_BiomeTans[selectedBiome].SettingsGUI();
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json BiomeGenerator::Serialize() const
{
	nlohmann::json serialized;

	serialized["biomeMapPxPerTile"] = m_BiomeMapPxPerTile;
	serialized["biomeBlending"] = m_BiomeBlending;

	serialized["seed"] = m_Seed;
	serialized["continentChance"] = m_ContinentChance;
	serialized["islandExpandChance"] = m_IslandExpandChance;
	serialized["islandErodeChance"] = m_IslandErodeChance;
	serialized["removeOceanChance"] = m_SmallIslandsChance;
	serialized["zoomingPerturbationChance"] = m_ZoomSamplePerturbation;

	serialized["temperateChance"] = m_TemperateBiomeChance;
	serialized["coldChance"] = m_ColdBiomeChance;
	serialized["warmChance"] = m_WarmBiomeChance;

	serialized["biomeMinimapColours"] = nlohmann::json::array();
	for (int i = 0; i < m_AllBiomes.size(); i++)
		serialized["biomeMinimapColours"].push_back(SerializationHelper::SerializeFloat4(m_BiomeMinimapColours[i]));

	serialized["generationSettings"] = nlohmann::json::array();
	for (int i = 0; i < m_AllBiomes.size(); i++)
		serialized["generationSettings"].push_back(m_GenerationSettings[i].Serialize());
	
	serialized["biomeTans"] = nlohmann::json::array();
	for (int i = 0; i < m_AllBiomes.size(); i++)
		serialized["biomeTans"].push_back(m_BiomeTans[i].Serialize());

	serialized["spawnWeights"] = nlohmann::json::object();
	for (auto& biome : m_AllBiomes)
		serialized["spawnWeights"][biome.name] = biome.spawnWeight;

	return serialized;
}

void BiomeGenerator::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("biomeMapPxPerTile")) m_BiomeMapPxPerTile = data["biomeMapPxPerTile"];
	if (data.contains("biomeBlending")) m_BiomeBlending = data["biomeBlending"];

	if (data.contains("seed")) m_Seed = data["seed"];
	if (data.contains("continentChance")) m_ContinentChance = data["continentChance"];
	if (data.contains("islandExpandChance")) m_IslandExpandChance = data["islandExpandChance"];
	if (data.contains("islandErodeChance")) m_IslandErodeChance = data["islandErodeChance"];
	if (data.contains("removeOceanChance")) m_SmallIslandsChance = data["removeOceanChance"];
	if (data.contains("zoomingPerturbationChance")) m_ZoomSamplePerturbation = data["zoomingPerturbationChance"];

	if (data.contains("temperateChance")) m_TemperateBiomeChance = data["temperateChance"];
	if (data.contains("coldChance")) m_ColdBiomeChance = data["coldChance"];
	if (data.contains("warmChance")) m_WarmBiomeChance = data["warmChance"];

	if (data.contains("biomeMinimapColours"))
	{
		int index = 0;
		for (auto& biome : data["biomeMinimapColours"])
		{
			SerializationHelper::LoadFloat4FromJson(m_BiomeMinimapColours + index, biome);
			index++;
		}
	}

	if (data.contains("generationSettings"))
	{
		int index = 0;
		for (auto& biome : data["generationSettings"])
		{
			m_GenerationSettings[index].LoadFromJson(biome);
			index++;
		}
	}

	if (data.contains("biomeTans"))
	{
		int index = 0;
		for (auto& biome : data["biomeTans"])
		{
			m_BiomeTans[index].LoadFromJson(biome);
			index++;
		}
	}

	if (data.contains("spawnWeights"))
	{
		auto& spawnWeights = data["spawnWeights"];
		for (auto& biome : m_AllBiomes)
		{
			if (spawnWeights.contains(biome.name))
				biome.spawnWeight = spawnWeights[biome.name];
		}
	}
}

void BiomeGenerator::GenerateBiomeMap(ID3D11Device* device)
{
	// clear out old biome map	
	if (m_BiomeMap) delete m_BiomeMap;
	if (m_BiomeMapSRV) m_BiomeMapSRV->Release();


	// reset world seed
	m_RNG.seed(m_Seed);

	// start with a really small biome map; 4x4
	m_BiomeMapSize = 4;
	m_BiomeMap = new int[m_BiomeMapSize * m_BiomeMapSize];

	// work out what will be land and what will be ocean
	IslandsCA(&m_BiomeMap, m_BiomeMapSize);
	Zoom2x(&m_BiomeMap, &m_BiomeMapSize);
	AddIslandsCA(&m_BiomeMap, m_BiomeMapSize);
	Zoom2x(&m_BiomeMap, &m_BiomeMapSize);
	AddIslandsCA(&m_BiomeMap, m_BiomeMapSize);
	AddIslandsCA(&m_BiomeMap, m_BiomeMapSize);
	AddIslandsCA(&m_BiomeMap, m_BiomeMapSize);
	RemoveTooMuchOcean(&m_BiomeMap, m_BiomeMapSize);
	
	// decide on biome temperatures
	size_t tempMapSize = m_BiomeMapSize;
	int* tempMap = new int[tempMapSize * tempMapSize];
	CreateTemperatures(&tempMap, m_BiomeMap, tempMapSize);
	
	Zoom2x(&m_BiomeMap, &m_BiomeMapSize);
	Zoom2x(&tempMap, &tempMapSize);
	
	TransitionTemperatures(&tempMap, tempMapSize);
	
	// now select biomes based off of the temperatures
	SelectBiomes(&m_BiomeMap, tempMap, m_BiomeMapSize);
	// temp map is no longer needed (temperatures have been assigned into biomes)
	delete[] tempMap;
	
	Zoom2x(&m_BiomeMap, &m_BiomeMapSize);
	
	AddShores(&m_BiomeMap, m_BiomeMapSize);

	CreateBiomeMapTexture(device);
}


void BiomeGenerator::IslandsCA(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int i = 0; i < mapSize * mapSize; i++)
	{
		if (Chance(m_ContinentChance))
			newBiomeMap[i] = BIOME_TYPE_LAND;
		else
			newBiomeMap[i] = BIOME_TYPE_OCEAN;
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::AddIslandsCA(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int sample = biomeMap[y * mapSize + x];
			
			if (sample == BIOME_TYPE_OCEAN)
			{
				// process oceans

				// if ocean is next to at least 1 land tile, then theres a chance it will also become land
				if (CountNeighboursEqual(x, y, BIOME_TYPE_LAND, biomeMap, mapSize) > 0)
				{
					if (Chance(m_IslandExpandChance))
						sample = BIOME_TYPE_LAND;
				}
			}
			else if (sample == BIOME_TYPE_LAND)
			{
				// process land
				// if land is next to more than 1 ocean tile, then theres a chance it will become ocean
				if (CountNeighboursEqual(x, y, BIOME_TYPE_OCEAN, biomeMap, mapSize) > 1)
				{
					if (Chance(m_IslandErodeChance))
						sample = BIOME_TYPE_OCEAN;
				}
			}

			newBiomeMap[y * mapSize + x] = sample;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::RemoveTooMuchOcean(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int sample = biomeMap[y * mapSize + x];

			if (CountNeighboursEqual(x, y, BIOME_TYPE_OCEAN, biomeMap, mapSize) == 8)
			{
				if (Chance(m_SmallIslandsChance))
					sample = BIOME_TYPE_LAND;
			}

			newBiomeMap[y * mapSize + x] = sample;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::CreateTemperatures(int** tempMapPtr, int* biomeMap, size_t mapSize)
{
	int* tempMap = *tempMapPtr;
	int* newTempMap = new int[mapSize * mapSize];

	int t = m_TemperateBiomeChance + m_WarmBiomeChance + m_ColdBiomeChance;
	std::uniform_int_distribution<int> temperatureDist(1, t);

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int biome = biomeMap[y * mapSize + x];
			int temperature = BIOME_TEMP_TEMPERATE;

			int r = temperatureDist(m_RNG);
			if (r <= m_TemperateBiomeChance)
				temperature = BIOME_TEMP_TEMPERATE;
			else if (r <= m_TemperateBiomeChance + m_ColdBiomeChance)
				temperature = BIOME_TEMP_COLD;
			else
				temperature = BIOME_TEMP_WARM;

			newTempMap[y * mapSize + x] = temperature;
		}
	}

	delete[] tempMap;
	*tempMapPtr = newTempMap;
}

void BiomeGenerator::TransitionTemperatures(int** tempMapPtr, size_t mapSize)
{
	int* tempMap = *tempMapPtr;
	int* newTempMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int temp = tempMap[y * mapSize + x];

			if (temp == BIOME_TEMP_COLD)
			{
				if (CountNeighboursEqual(x, y, BIOME_TEMP_WARM, tempMap, mapSize) > 3)
				{
					temp = BIOME_TEMP_TEMPERATE;
				}
			} 
			else if(temp == BIOME_TEMP_WARM)
			{
				if (CountNeighboursEqual(x, y, BIOME_TEMP_COLD, tempMap, mapSize) > 3)
				{
					temp = BIOME_TEMP_TEMPERATE;
				}
			}

			newTempMap[y * mapSize + x] = temp;
		}
	}

	delete[] tempMap;
	*tempMapPtr = newTempMap;
}

void BiomeGenerator::SelectBiomes(int** biomeMapPtr, int* tempMap, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			BIOME_TYPE biomeType = static_cast<BIOME_TYPE>(biomeMap[y * mapSize + x]);
			BIOME_TEMP biomeTemp = static_cast<BIOME_TEMP>(tempMap[y * mapSize + x]);
			int biome;

			// select biomes
			const std::vector<int>& candidates = biomeType == BIOME_TYPE_OCEAN ? 
				m_SpawnableOceanBiomesByTemp[biomeTemp] :
				m_SpawnableLandBiomesByTemp[biomeTemp];
			float totalOdds = 0.0f;
			for (auto index : candidates)
				totalOdds += m_AllBiomes[index].spawnWeight;

			int chance = m_Chance(m_RNG);
			int p = 0;
			for (auto index : candidates)
			{
				int scaledOdds = static_cast<int>(ceil(100.0f * m_AllBiomes[index].spawnWeight / totalOdds));
				p += scaledOdds;
				if (chance <= p)
				{
					biome = index;
					break;
				}
			}

			newBiomeMap[y * mapSize + x] = biome;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::AddShores(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int biome = biomeMap[y * mapSize + x];

			// select biomes
			if (m_AllBiomes[biome].type == BIOME_TYPE_LAND)
			{
				if (CountNeighboursEqual(x, y, [this](int biome) { return m_AllBiomes[biome].type == BIOME_TYPE_OCEAN; }, biomeMap, mapSize) > 0)
				{
					if (m_AllBiomes[biome].temperature == BIOME_TEMP_COLD)
						biome = GetBiomeIDByName("Cold Shore");
					else
						biome = GetBiomeIDByName("Shore");
				}
			}

			newBiomeMap[y * mapSize + x] = biome;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::Zoom2x(int** mapPtr, size_t* mapSize)
{
	int* map = *mapPtr;
	size_t oldSize = (*mapSize);

	size_t newSize = 2 * oldSize;
	int* newMap = new int[newSize * newSize];

	for (int y = 0; y < newSize; y++)
	{
		for (int x = 0; x < newSize; x++)
		{
			int sampleX = x / 2;
			int sampleY = y / 2;

			if (Chance(m_ZoomSamplePerturbation))
			{
				if (Chance(50))
					sampleX += Chance(50) ? 1 : -1;
				else
					sampleY += Chance(50) ? 1 : -1;
			}
			sampleX = max(0, sampleX);
			sampleX = min(static_cast<int>(oldSize - 1), sampleX);
			sampleY = max(0, sampleY);
			sampleY = min(static_cast<int>(oldSize - 1), sampleY);

			int sample = map[sampleY * oldSize + sampleX];

			newMap[y * newSize + x] = sample;
		}
	}

	delete[] map;
	*mapPtr = newMap;
	*mapSize = newSize;
}


int BiomeGenerator::GetBiomeIDByName(const char* name) const
{
	int index = 0;
	for (auto& biome : m_AllBiomes)
	{
		if (strcmp(name, biome.name) == 0)
			return index;
		index++;
	}
	assert(false && "No biome found with that name");
	return -1;
}

int BiomeGenerator::CountNeighboursEqual(int x, int y, int v, int* biomeMap, size_t mapSize)
{
	return CountNeighboursEqual(x, y, [v](int biome) { return biome == v; }, biomeMap, mapSize);
}

int BiomeGenerator::CountNeighboursEqual(int x, int y, std::function<bool(int biome)> condition, int* biomeMap, size_t mapSize)
{
	int count = 0;

	if (x > 0)
	{
		count += condition(biomeMap[y		* mapSize + (x - 1)]) ? 1 : 0;

		if (y > 0)
			count += condition(biomeMap[(y - 1) * mapSize + (x - 1)]) ? 1 : 0;
		if (y < mapSize - 1)
			count += condition(biomeMap[(y + 1) * mapSize + (x - 1)]) ? 1 : 0;
	}

	if (y > 0)
		count += condition(biomeMap[(y - 1) * mapSize + x	   ]) ? 1 : 0;
	if (y < mapSize - 1)
		count += condition(biomeMap[(y + 1) * mapSize + x	   ]) ? 1 : 0;


	if (x < mapSize - 1)
	{
		count += condition(biomeMap[y * mapSize + (x + 1)]) ? 1 : 0;

		if (y > 0)
			count += condition(biomeMap[(y - 1) * mapSize + (x + 1)]) ? 1 : 0;
		if (y < mapSize - 1)
			count += condition(biomeMap[(y + 1) * mapSize + (x + 1)]) ? 1 : 0;
	}

	return count;
}


void BiomeGenerator::CreateBiomeMapTexture(ID3D11Device* device)
{
	if (!m_BiomeMap) return;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = static_cast<unsigned int>(m_BiomeMapSize);
	desc.Height = static_cast<unsigned int>(m_BiomeMapSize);
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_SINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = m_BiomeMap;
	initialData.SysMemPitch = static_cast<unsigned int>(sizeof(int) * m_BiomeMapSize);
	initialData.SysMemSlicePitch = 0;

	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = device->CreateTexture2D(&desc, &initialData, &tex);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_SINT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(tex, &srvDesc, &m_BiomeMapSRV);
	assert(hr == S_OK);
}

void BiomeGenerator::CreateBiomeMappingBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(BiomeMappingBufferType);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	BiomeMappingBufferType bmbt{ 
		m_BiomeMapPxPerTile, 
		static_cast<unsigned int>(m_BiomeMapSize),
		m_BiomeBlending,
		0.0f
	};
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &bmbt;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &initialData, &m_BiomeMappingBuffer);
	assert(hr == S_OK);
}

void BiomeGenerator::CreateGenerationSettingsBuffer(ID3D11Device* device)
{
	// create biome data buffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(TerrainNoiseSettings) * MAX_BIOMES;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(TerrainNoiseSettings);

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &m_GenerationSettings;
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &initialData, &m_GenerationSettingsBuffer);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = MAX_BIOMES;

	hr = device->CreateShaderResourceView(m_GenerationSettingsBuffer, &srvDesc, &m_GenerationSettingsView);
	assert(hr == S_OK);
}

void BiomeGenerator::CreateBiomeTanBuffer(ID3D11Device* device)
{
	// create biome data buffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(BiomeTan) * MAX_BIOMES;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(BiomeTan);

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &m_BiomeTans;
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, &initialData, &m_BiomeTanBuffer);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = MAX_BIOMES;

	hr = device->CreateShaderResourceView(m_BiomeTanBuffer, &srvDesc, &m_BiomeTanView);
	assert(hr == S_OK);
}

void BiomeGenerator::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = deviceContext->Map(m_GenerationSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	memcpy(mappedResource.pData, &m_GenerationSettings, sizeof(TerrainNoiseSettings) * MAX_BIOMES);
	deviceContext->Unmap(m_GenerationSettingsBuffer, 0);

	hr = deviceContext->Map(m_BiomeTanBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	memcpy(mappedResource.pData, &m_BiomeTans, sizeof(BiomeTan) * MAX_BIOMES);
	deviceContext->Unmap(m_BiomeTanBuffer, 0);

	hr = deviceContext->Map(m_BiomeMappingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	BiomeMappingBufferType* dataPtr = reinterpret_cast<BiomeMappingBufferType*>(mappedResource.pData);
	dataPtr->pxPerTile = m_BiomeMapPxPerTile;
	dataPtr->resolution = static_cast<unsigned int>(m_BiomeMapSize);
	dataPtr->blending = m_BiomeBlending;
	deviceContext->Unmap(m_BiomeMappingBuffer, 0);
}


const char* BiomeGenerator::StrFromBiomeType(BIOME_TYPE type)
{
	switch (type)
	{
	case BiomeGenerator::BIOME_TYPE_OCEAN:	return "Ocean";
	case BiomeGenerator::BIOME_TYPE_LAND:	return "Land";
	default:				return "Unknown";
	}
}
const char* BiomeGenerator::StrFromBiomeTemp(BIOME_TEMP temp)
{
	switch (temp)
	{
	case BiomeGenerator::BIOME_TEMP_TEMPERATE:	return "Temperate";
	case BiomeGenerator::BIOME_TEMP_COLD:		return "Cold";
	case BiomeGenerator::BIOME_TEMP_WARM:		return "Warm";
	default:					return "Unknown";
	}
}

#include "BiomeGenerator.h"

#include <algorithm>
#include <cassert>

#include "imGUI/imgui.h"
#include "SerializationHelper.h"

BiomeGenerator::BiomeGenerator(unsigned int seed)
	: m_RNG(seed), m_Chance(1, 100)
{
	// create biomes
	m_AllBiomes.clear();
	m_AllBiomes.push_back({ "Ocean",			BIOME_TYPE_OCEAN, BIOME_TEMP_TEMPERATE, false, 1 });
	m_AllBiomes.push_back({ "Plains",			BIOME_TYPE_LAND,  BIOME_TEMP_TEMPERATE, false, 1 });
	m_AllBiomes.push_back({ "Mountains",		BIOME_TYPE_LAND,  BIOME_TEMP_TEMPERATE, true,  0 });
	m_AllBiomes.push_back({ "Desert",			BIOME_TYPE_LAND,  BIOME_TEMP_WARM,		false, 0 });
	m_AllBiomes.push_back({ "Desert Hills",		BIOME_TYPE_LAND,  BIOME_TEMP_WARM,		true,  0 });
	m_AllBiomes.push_back({ "Tundra",			BIOME_TYPE_LAND,  BIOME_TEMP_COLD,		false, 0 });
	m_AllBiomes.push_back({ "Snowy Mountains",	BIOME_TYPE_LAND,  BIOME_TEMP_COLD,		true,  0 });

	// process biome list
	GetAllBiomesWithTemperature(m_ColdBiomes, BIOME_TEMP_COLD);
	GetAllBiomesWithTemperature(m_TemperateBiomes, BIOME_TEMP_TEMPERATE);
	GetAllBiomesWithTemperature(m_WarmBiomes, BIOME_TEMP_WARM);

	m_BiomesByTemp.insert(std::make_pair( BIOME_TEMP_COLD, &m_ColdBiomes ));
	m_BiomesByTemp.insert(std::make_pair( BIOME_TEMP_TEMPERATE, &m_TemperateBiomes ));
	m_BiomesByTemp.insert(std::make_pair( BIOME_TEMP_WARM, &m_WarmBiomes ));
}

BiomeGenerator::~BiomeGenerator()
{
	if (m_BiomeMap) delete m_BiomeMap;
	if (m_TemperatureMap) delete m_TemperatureMap;
}

bool BiomeGenerator::SettingsGUI()
{
	bool changed = false;

	ImGui::Separator();

	ImGui::Text("Biome map size: %d", m_BiomeMapSize);
	ImGui::Text("Mapping:");
	changed |= ImGui::DragFloat2("Top Left", &m_BiomeMapTopLeft.x, 0.01f);
	changed |= ImGui::DragFloat("Scale", &m_BiomeMapScale, 0.01f);
	changed |= ImGui::SliderFloat("Blending", &m_BiomeBlending, 0.01f, 1.0f);
	
	ImGui::Separator();
	ImGui::Text("Biome Settings:");

	static int selectedBiome = 0;
	struct FuncHolder { // to allow inline function declaration
		static bool ItemGetter(void* data, int idx, const char** out_str)
		{
			*out_str = ((Biome*)data + idx)->name;
			return true;
		}
	};

	ImGui::Combo("Biome", &selectedBiome, &FuncHolder::ItemGetter, m_AllBiomes.data(), static_cast<int>(m_AllBiomes.size()));

	Biome& biome = m_AllBiomes.at(selectedBiome);
	ImGui::Separator();
	ImGui::Text("%s Biome Properties:", biome.name);
	ImGui::Text("Type: %s", StrFromBiomeType(biome.type));
	ImGui::Text("Temperature: %s", StrFromBiomeTemp(biome.temperature));
	ImGui::Text("Spawn weight: %d", biome.spawnWeight);

	ImGui::Separator();
	ImGui::Text("Heightmap Generation:");
	changed |= m_GenerationSettings[selectedBiome].SettingsGUI();

	ImGui::Separator();

	return changed;
}

nlohmann::json BiomeGenerator::Serialize() const
{
	nlohmann::json serialized;

	serialized["biomeMapTopLeft"] = SerializationHelper::SerializeFloat2(m_BiomeMapTopLeft);
	serialized["biomeMapScale"] = m_BiomeMapScale;
	serialized["biomeBlending"] = m_BiomeBlending;

	serialized["generationSettings"] = nlohmann::json::array();
	for (int i = 0; i < m_AllBiomes.size(); i++)
	{
		serialized["generationSettings"].push_back(m_GenerationSettings[i].Serialize());
	}

	return serialized;
}

void BiomeGenerator::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("biomeMapTopLeft")) SerializationHelper::LoadFloat2FromJson(&m_BiomeMapTopLeft, data["biomeMapTopLeft"]);
	if (data.contains("biomeMapScale")) m_BiomeMapScale = data["biomeMapScale"];
	if (data.contains("biomeBlending")) m_BiomeBlending = data["biomeBlending"];

	if (data.contains("generationSettings"))
	{
		int index = 0;
		for (auto& biome : data["generationSettings"])
		{
			m_GenerationSettings[index].LoadFromJson(biome);
			index++;
		}
	}
}

void BiomeGenerator::GenerateBiomeMap(ID3D11Device* device)
{
	if (m_BiomeMap) delete m_BiomeMap;
	if (m_TemperatureMap) delete m_TemperatureMap;

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
	m_TemperatureMapSize = m_BiomeMapSize;
	m_TemperatureMap = new int[m_TemperatureMapSize * m_TemperatureMapSize];

	CreateTemperatures(&m_TemperatureMap, m_BiomeMap, m_TemperatureMapSize);
	TransitionTemperatures(&m_TemperatureMap, m_TemperatureMapSize);

	// now select biomes based off of the temperatures
	SelectBiomes(&m_BiomeMap, m_TemperatureMap, m_BiomeMapSize);


	// create resources used by GPU to generate the terrain
	CreateBiomeMapTexture(device);
	CreateGenerationSettingsBuffer(device);
}


void BiomeGenerator::IslandsCA(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int i = 0; i < mapSize * mapSize; i++)
	{
		if (Chance(20))
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
					if (Chance(30))
						sample = BIOME_TYPE_LAND;
				}
			}
			else if (sample == BIOME_TYPE_LAND)
			{
				// process land
				// if land is next to more than 1 ocean tile, then theres a chance it will become ocean
				if (CountNeighboursEqual(x, y, BIOME_TYPE_OCEAN, biomeMap, mapSize) > 1)
				{
					if (Chance(15))
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

			if (CountNeighboursEqual(x, y, BIOME_TYPE_OCEAN, biomeMap, mapSize) == 4)
			{
				if (Chance(25))
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

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int biome = biomeMap[y * mapSize + x];
			int temperature = BIOME_TEMP_TEMPERATE;

			if (biome != BIOME_TYPE_OCEAN)
			{
				int r = m_Chance(m_RNG);
				//if (r <= 66)
				if (true)
					temperature = BIOME_TEMP_TEMPERATE;
				else if (r <= 83)
					temperature = BIOME_TEMP_COLD;
				else
					temperature = BIOME_TEMP_WARM;
			}

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
				if (CountNeighboursEqual(x, y, BIOME_TEMP_WARM, tempMap, mapSize) > 0)
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
			if (biomeType == BIOME_TYPE_OCEAN)
				biome = GetBiomeIDByName("Ocean");
			else
			{
				const std::vector<int>& candidates = *m_BiomesByTemp[biomeTemp];
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
			}

			newBiomeMap[y * mapSize + x] = biome;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::Zoom2x(int** biomeMapPtr, size_t* mapSize)
{
	int* biomeMap = *biomeMapPtr;
	size_t oldSize = (*mapSize);

	size_t newSize = 2 * oldSize;
	int* newBiomeMap = new int[newSize * newSize];

	for (int y = 0; y < newSize; y++)
	{
		for (int x = 0; x < newSize; x++)
		{
			int sampleX = x / 2;
			int sampleY = y / 2;

			int sample = biomeMap[sampleY * oldSize + sampleX];

			if (Chance(20))
			{
				if (CountNeighboursEqual(sampleX, sampleY, BIOME_TYPE_LAND, biomeMap, oldSize) > 0)
					sample = sample == BIOME_TYPE_LAND ? BIOME_TYPE_OCEAN : BIOME_TYPE_LAND;
			}

			newBiomeMap[y * newSize + x] = sample;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
	*mapSize = newSize;
}

void BiomeGenerator::GetAllBiomesWithTemperature(std::vector<int>& out, BIOME_TEMP temperature)
{
	out.clear();
	int index = 0;
	for (const auto& biome : m_AllBiomes)
	{
		if (biome.temperature == temperature && biome.type == BIOME_TYPE_LAND)
			out.push_back(index);
		index++;
	}
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
	int count = 0;
	
	if (x > 0)
		count += biomeMap[y * mapSize + (x - 1)] == v;
	if (x < mapSize - 1)
		count += biomeMap[y * mapSize + (x + 1)] == v;
	if (y > 0)
		count += biomeMap[(y - 1) * mapSize + x] == v;
	if (y < mapSize - 1)
		count += biomeMap[(y + 1) * mapSize + x] == v;

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

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(BiomeMappingBufferType);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	BiomeMappingBufferType bmbt{ 
		m_BiomeMapTopLeft, 
		m_BiomeMapScale, 
		static_cast<unsigned int>(m_BiomeMapSize),
		m_BiomeBlending,
		{ 0.0f, 0.0f, 0.0f }
	};
	initialData.pSysMem = &bmbt;

	hr = device->CreateBuffer(&bufferDesc, &initialData, &m_BiomeMappingBuffer);
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

void BiomeGenerator::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = deviceContext->Map(m_GenerationSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	memcpy(mappedResource.pData, &m_GenerationSettings, sizeof(TerrainNoiseSettings) * MAX_BIOMES);
	deviceContext->Unmap(m_GenerationSettingsBuffer, 0);

	hr = deviceContext->Map(m_BiomeMappingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	BiomeMappingBufferType* dataPtr = reinterpret_cast<BiomeMappingBufferType*>(mappedResource.pData);
	dataPtr->topleft = m_BiomeMapTopLeft;
	dataPtr->scale = m_BiomeMapScale;
	dataPtr->resolution = static_cast<unsigned int>(m_BiomeMapSize);
	dataPtr->blending = m_BiomeBlending;
	deviceContext->Unmap(m_BiomeMappingBuffer, 0);
}


const char* BiomeGenerator::StrFromBiomeType(BIOME_TYPE type)
{
	switch (type)
	{
	case BIOME_TYPE_OCEAN:	return "Ocean";
	case BIOME_TYPE_LAND:	return "Land";
	default:				return "Unknown";
	}
}
const char* BiomeGenerator::StrFromBiomeTemp(BIOME_TEMP temp)
{
	switch (temp)
	{
	case BIOME_TEMP_TEMPERATE:	return "Temperate";
	case BIOME_TEMP_COLD:		return "Cold";
	case BIOME_TEMP_WARM:		return "Warm";
	default:					return "Unknown";
	}
}

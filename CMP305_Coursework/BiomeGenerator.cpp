#include "BiomeGenerator.h"

#include <algorithm>
#include <cassert>


BiomeGenerator::BiomeGenerator(unsigned int seed)
	: m_RNG(seed), m_Chance(1, 100)
{
}

BiomeGenerator::~BiomeGenerator()
{
}

void BiomeGenerator::GenerateBiomeMap()
{
	int* biomeMap = nullptr;

	// start with a really small biome map; 4x4
	size_t mapSize = 4;
	biomeMap = new int[mapSize * mapSize];

	// work out what will be land and what will be ocean
	IslandsCA(&biomeMap, mapSize);
	Zoom2x(&biomeMap, &mapSize);
	AddIslandsCA(&biomeMap, mapSize);
	Zoom2x(&biomeMap, &mapSize);
	AddIslandsCA(&biomeMap, mapSize);
	AddIslandsCA(&biomeMap, mapSize);
	AddIslandsCA(&biomeMap, mapSize);
	RemoveTooMuchOcean(&biomeMap, mapSize);

	// decide on biome temperatures
	CreateTemperatures(&biomeMap, mapSize);
	TransitionTemperatures(&biomeMap, mapSize);

	// temperature transitions

	m_BiomeMap = biomeMap;
	m_BiomeMapSize = mapSize;
}

void BiomeGenerator::CreateDebugTexture(ID3D11Device* device)
{
	if (!m_BiomeMap) return;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width =  static_cast<unsigned int>(m_BiomeMapSize);
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

	D3D11_SUBRESOURCE_DATA pData;
	pData.pSysMem = m_BiomeMap;
	pData.SysMemPitch = static_cast<unsigned int>(sizeof(int) * m_BiomeMapSize);
	pData.SysMemSlicePitch = 0;

	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = device->CreateTexture2D(&desc, &pData, &tex);
	assert(hr == S_OK);
}

void BiomeGenerator::IslandsCA(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int i = 0; i < mapSize * mapSize; i++)
	{
		newBiomeMap[i] = Chance(20);
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
			
			if (sample == 0)
			{
				// process oceans

				// if ocean is next to at least 1 land tile, then theres a chance it will also become land
				if (CountNeighboursEqual(x, y, 1, biomeMap, mapSize) > 0)
				{
					if (Chance(30))
						sample = 1;
				}
			}
			else if (sample == 1)
			{
				// process land
				// if land is next to at least 1 ocean tile, then theres a chance it will become ocean
				if (CountNeighboursEqual(x, y, 0, biomeMap, mapSize) == 1)
				{
					if (Chance(15))
						sample = 0;
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

			if (CountNeighboursEqual(x, y, 0, biomeMap, mapSize) == 4)
			{
				if (Chance(25))
					sample = 1;
			}

			newBiomeMap[y * mapSize + x] = sample;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::CreateTemperatures(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int sample = biomeMap[y * mapSize + x];

			if (sample > 0)
			{
				int r = m_Chance(m_RNG);
				if (r <= 66)
					sample = 1;
				else if (r <= 83)
					sample = 2;
				else
					sample = 3;
			}

			newBiomeMap[y * mapSize + x] = sample;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
}

void BiomeGenerator::TransitionTemperatures(int** biomeMapPtr, size_t mapSize)
{
	int* biomeMap = *biomeMapPtr;
	int* newBiomeMap = new int[mapSize * mapSize];

	for (int y = 0; y < mapSize; y++)
	{
		for (int x = 0; x < mapSize; x++)
		{
			int sample = biomeMap[y * mapSize + x];

			newBiomeMap[y * mapSize + x] = sample;
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
				if (CountNeighboursEqual(sampleX, sampleY, 1, biomeMap, oldSize) > 0)
					sample = 1 - sample;
			}

			newBiomeMap[y * newSize + x] = sample;
		}
	}

	delete[] biomeMap;
	*biomeMapPtr = newBiomeMap;
	*mapSize = newSize;
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

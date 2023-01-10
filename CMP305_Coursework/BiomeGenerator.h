#pragma once

#include <d3d11.h>
#include <random>


class BiomeGenerator 
{
public:
	BiomeGenerator(unsigned int seed);
	~BiomeGenerator();

	void GenerateBiomeMap();

	void CreateDebugTexture(ID3D11Device* device);

private:
	// land/ocean balance
	void IslandsCA(int** biomeMapPtr, size_t mapSize);
	void AddIslandsCA(int** biomeMapPtr, size_t mapSize);
	void RemoveTooMuchOcean(int** biomeMapPtr, size_t mapSize);

	// biome temperatures
	void CreateTemperatures(int** biomeMapPtr, size_t mapSize);
	void TransitionTemperatures(int** biomeMapPtr, size_t mapSize);

	// zoom
	void Zoom2x(int** biomeMapPtr, size_t* mapSize);

	// utility
	int CountNeighboursEqual(int x, int y, int v, int* biomeMap, size_t mapSize);

	inline bool Chance(int percent) { return m_Chance(m_RNG) <= percent; }

private:
	std::mt19937 m_RNG;
	std::uniform_int_distribution<int> m_Chance;

	int* m_BiomeMap = nullptr;
	size_t m_BiomeMapSize = -1;
};
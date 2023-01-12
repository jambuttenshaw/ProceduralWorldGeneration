// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include <d3d11.h>

#include "LightShader.h"
#include "TerrainShader.h"
#include "WaterShader.h"
#include "BiomeMapShader.h"

#include "RenderTarget.h"

#include "TerrainMesh.h"
#include "Heightmap.h"

#include "GameObject.h"

#include <array>

class HeightmapFilter;
class BiomeGenerator;


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

	// passes
	void worldPass();
	void waterPass();

	void applyFilterStack();

	void createTerrain(const XMFLOAT3& pos);

	void saveSettings(const std::string& file);
	void loadSettings(const std::string& file);

private:
	float m_Time = 0.0f;

	LightShader* m_LightShader = nullptr;
	TerrainShader* m_TerrainShader = nullptr;
	WaterShader* m_WaterShader = nullptr;
	BiomeMapShader* m_BiomeMapShader = nullptr;

	RenderTarget* m_RenderTarget = nullptr;

	TerrainMesh* m_TerrainMesh = nullptr;
	CubeMesh* m_Cube = nullptr;
	// for debug display of textures
	OrthoMesh* m_OrthoMesh = nullptr;

	std::vector<GameObject> m_GameObjects;
	XMFLOAT2 m_WorldMinPos{ 0, 0 };
	XMFLOAT2 m_WorldSize{ 0, 0 };

	std::vector<Heightmap*> m_Heightmaps;
	std::map<size_t, Heightmap*> m_GOToHeightmap;
	
	Light* light;
	XMFLOAT3 lightDiffuse{ 1.0f, 1.0f, 1.0f };
	XMFLOAT3 lightAmbient{ 0.2f, 0.2f, 0.2f };
	XMFLOAT3 lightSpecular{ 0.2f, 0.2f, 0.2f };
	XMFLOAT3 lightDir{ 0.7f, -0.7f, 0.7f };

	BiomeGenerator* m_BiomeGenerator = nullptr;
	HeightmapFilter* m_HeightmapFilter = nullptr;
	
	char m_SaveFilePath[128];
	bool m_LoadOnOpen = true;
	bool m_SaveOnExit = false;
};

#endif
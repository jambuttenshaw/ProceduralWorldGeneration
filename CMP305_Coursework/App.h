// Application.h
#ifndef _APP_H
#define _APP_H

// Includes
#include "DXF.h"	// include dxframework
#include <wrl/client.h>
#include <memory>

#include "TerrainMesh.h"
#include "LightShader.h"
#include "InstanceShader.h"
#include "UnlitShader.h"
#include "InstancedCubeMesh.h"
#include "QuadMeshT.h"
#include "WritableTexture.h"
#include "LineMesh.h"

using std::unique_ptr;

class App : public BaseApplication
{
public:

	App();
	~App();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	void BuildCubeInstances();

private:
	unique_ptr<InstanceShader>		m_InstanceShader;
	unique_ptr<UnlitShader>			m_UnlitShader;
	unique_ptr<LightShader>			m_LightShader;

	unique_ptr<InstancedCubeMesh>	m_InstancedCube;
	unique_ptr<QuadMeshT>			m_Quad;
	unique_ptr<TerrainMesh>			m_Terrain;
	unique_ptr<LineMesh>			m_Line;

	unique_ptr<Light>				m_Light;

	unique_ptr<WritableTexture>		m_Texture;

	int m_TerrainResolution;
};

#endif
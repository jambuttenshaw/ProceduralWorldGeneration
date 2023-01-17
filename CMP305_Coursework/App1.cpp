#include "App1.h"

#include <nlohmann/json.hpp>
#include <set>
#include <queue>

#include "HeightmapFilter.h"
#include "SerializationHelper.h"

#include "BiomeGenerator.h"


App1::App1()
{
	m_TerrainMesh = nullptr;
	m_TerrainShader = nullptr;

	strcpy_s(m_SaveFilePath, "res/settings/alien.json");
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"oceanNormalMapA", L"res/wave_normals1.png");
	textureMgr->loadTexture(L"oceanNormalMapB", L"res/wave_normals2.png");

	m_LightShader = new LightShader(renderer->getDevice(), hwnd);
	m_TerrainShader = new TerrainShader(renderer->getDevice());
	m_WaterShader = new WaterShader(renderer->getDevice(), textureMgr->getTexture(L"oceanNormalMapA"), textureMgr->getTexture(L"oceanNormalMapB"));
	m_BiomeMapShader = new BiomeMapShader(renderer->getDevice(), hwnd);

	m_RenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_TerrainMesh = new TerrainMesh(renderer->getDevice(), 128, m_TileSize);
	m_Cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext(), 2);
	m_OrthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 200, 200, (screenWidth / 2) - 100, (screenHeight / 2) - 100);

	camera->setPosition(150.0f, 90.0f, 150.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	XMFLOAT3 cameraPos = camera->getPosition();
	m_OldTile = {
		static_cast<int>(floor(cameraPos.x / m_TileSize)),
		static_cast<int>(floor(cameraPos.z / m_TileSize))
	};

	// create game objects
	updateTerrainGOs();

	// Initialise light
	light = new Light();
	light->setDiffuseColour(lightDiffuse.x, lightDiffuse.y, lightDiffuse.z, 1.0f);
	light->setAmbientColour(lightAmbient.x, lightAmbient.y, lightAmbient.z, 1.0f);
	light->setSpecularColour(lightSpecular.x, lightSpecular.y, lightSpecular.z, 1.0f);
	light->setDirection(lightDir.x, lightDir.y, lightDir.z);

	m_BiomeGenerator = new BiomeGenerator(renderer->getDevice(), 1);
	m_BiomeGenerator->GenerateBiomeMap(renderer->getDevice());
	m_HeightmapFilter = new HeightmapFilter(renderer->getDevice(), L"terrainNoise_cs.cso");

	if (m_LoadOnOpen)
	{
		loadSettings(std::string(m_SaveFilePath));
		m_BiomeGenerator->GenerateBiomeMap(renderer->getDevice());
		regenerateAllHeightmaps();
	}
}


App1::~App1()
{
	// probably not the best place to put this...
	if (m_SaveOnExit) saveSettings(std::string(m_SaveFilePath));


	if (m_TerrainMesh) delete m_TerrainMesh;
	if (m_TerrainShader) delete m_TerrainShader;
	if (m_LightShader) delete m_LightShader;

	if (m_RenderTarget) delete m_RenderTarget;

	if (m_HeightmapFilter) delete m_HeightmapFilter;

	for (auto heightmap : m_Heightmaps)
		delete heightmap.second;
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	m_Time += timer->getTime();


	// update the loaded terrains
	XMFLOAT3 cameraPos = camera->getPosition();
	XMINT2 worldTile = {
		static_cast<int>(floor(cameraPos.x / m_TileSize)),
		static_cast<int>(floor(cameraPos.z / m_TileSize))
	};
	if (worldTile.x != m_OldTile.x || worldTile.y != m_OldTile.y)
	{
		updateTerrainGOs();
	}
	m_OldTile = worldTile;


	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();
	m_BiomeGenerator->UpdateBuffers(renderer->getDeviceContext());


	// render world to render texture
	m_RenderTarget->Clear(renderer->getDeviceContext(), { 0.39f, 0.58f, 0.92f, 1.0f });
	if (!wireframeToggle) m_RenderTarget->Set(renderer->getDeviceContext());
	worldPass();

	if (!wireframeToggle)
	{
		// water is a post-processing effect and rendered afterwards
		renderer->setBackBufferRenderTarget();
		
		waterPass();
	}

	renderer->setZBuffer(false);
	if (m_BiomeGenerator->ShowBiomeMap())
	{
		m_OrthoMesh->sendData(renderer->getDeviceContext());

		XMMATRIX worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();

		XMFLOAT3 cameraPos = camera->getPosition();
		XMFLOAT2 worldTile = { floor(cameraPos.x / m_TileSize), floor(cameraPos.z / m_TileSize) };

		m_BiomeMapShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_BiomeGenerator, worldTile, m_ViewSize);
		m_BiomeMapShader->render(renderer->getDeviceContext(), m_OrthoMesh->getIndexCount());
	}
	renderer->setZBuffer(true);

	// GUI
	
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build GUI
	gui();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::worldPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();


	int goIndex = 0;
	for (auto& go : m_GameObjects)
	{
		XMMATRIX w = worldMatrix * go->transform.GetMatrix();
		switch (go->meshType)
		{
		case GameObject::MeshType::Regular:
		{
			go->mesh.regular->sendData(renderer->getDeviceContext());
			m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, light);
			m_LightShader->render(renderer->getDeviceContext(), go->mesh.regular->getIndexCount());
			break;
		}
		case GameObject::MeshType::Terrain:
		{
			// calculate heightmap index
			int x = static_cast<int>(floor(go->transform.GetTranslation().x / go->mesh.terrain->GetSize()));
			int y = static_cast<int>(floor(go->transform.GetTranslation().z / go->mesh.terrain->GetSize()));

			go->mesh.terrain->SendData(renderer->getDeviceContext());
			m_TerrainShader->SetShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Heightmaps.at({ x, y }), m_BiomeGenerator, light);
			m_TerrainShader->Render(renderer->getDeviceContext(), go->mesh.terrain->GetIndexCount());
			break;
		}
		}
		goIndex++;
	}
}

void App1::waterPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	{

		renderer->setZBuffer(false);
		m_WaterShader->setShaderParameters(renderer->getDeviceContext(), viewMatrix, projectionMatrix,
			m_RenderTarget->GetColourSRV(), m_RenderTarget->GetDepthSRV(),
			light, camera, m_Time, m_BiomeGenerator,
			m_OldTile, m_ViewSize, m_TileSize);
		m_WaterShader->Render(renderer->getDeviceContext());
		renderer->setZBuffer(true);
	}
}

void App1::gui()
{
	bool regenerateTerrain = false;

	if (ImGui::CollapsingHeader("General"))
	{
		ImGui::Text("FPS: %.2f", timer->getFPS());
		ImGui::Checkbox("Wireframe mode", &wireframeToggle);
		ImGui::Separator();

		if (ImGui::TreeNode("Camera"))
		{
			XMFLOAT3 camPos = camera->getPosition();
			if (ImGui::DragFloat3("Camera Pos", &camPos.x, 1.0f))
				camera->setPosition(camPos.x, camPos.y, camPos.z);
			XMFLOAT3 camRot = camera->getRotation();
			if (ImGui::DragFloat3("Camera Rot", &camRot.x, 0.5f))
				camera->setRotation(camRot.x, camRot.y, camRot.z);

			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Lighting"))
		{
			if (ImGui::ColorEdit3("Light Diffuse", &lightDiffuse.x))
				light->setDiffuseColour(lightDiffuse.x, lightDiffuse.y, lightDiffuse.z, 1.0f);
			if (ImGui::ColorEdit3("Light Ambient", &lightAmbient.x))
				light->setAmbientColour(lightAmbient.x, lightAmbient.y, lightAmbient.z, 1.0f);
			if (ImGui::ColorEdit3("Light Specular", &lightSpecular.x))
				light->setSpecularColour(lightSpecular.x, lightSpecular.y, lightSpecular.z, 1.0f);
			if (ImGui::SliderFloat3("Light Direction", &lightDir.x, -1.0f, 1.0f))
				light->setDirection(lightDir.x, lightDir.y, lightDir.z);

			ImGui::TreePop();
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Save/Load Settings"))
	{
		ImGui::InputText("Save file", m_SaveFilePath, IM_ARRAYSIZE(m_SaveFilePath));

		if (ImGui::Button("Save"))
			saveSettings(std::string(m_SaveFilePath));
		ImGui::SameLine();
		if (ImGui::Button("Open"))
		{
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}

		ImGui::Checkbox("Load On Open", &m_LoadOnOpen);
		ImGui::Checkbox("Save On Exit", &m_SaveOnExit);

		ImGui::Separator();
		ImGui::Text("Load Examples:");

		if (ImGui::Button("Earth"))
		{
			strcpy_s(m_SaveFilePath, "res/settings/earth.json");
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}
		if (ImGui::Button("Pangaea"))
		{
			strcpy_s(m_SaveFilePath, "res/settings/pangaea.json");
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}
		if (ImGui::Button("Archipelago"))
		{
			strcpy_s(m_SaveFilePath, "res/settings/archipelago.json");
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}
		if (ImGui::Button("Icy Lands"))
		{
			strcpy_s(m_SaveFilePath, "res/settings/icy lands.json");
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}
		if (ImGui::Button("Wild West"))
		{
			strcpy_s(m_SaveFilePath, "res/settings/wild west.json");
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}
		if (ImGui::Button("Alien"))
		{
			strcpy_s(m_SaveFilePath, "res/settings/alien.json");
			loadSettings(std::string(m_SaveFilePath));
			regenerateTerrain = true;
		}
		if (regenerateTerrain) m_BiomeGenerator->GenerateBiomeMap(renderer->getDevice());
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ocean"))
	{
		m_WaterShader->SettingsGUI();
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Terrain"))
	{
		ImGui::Separator();

		if (ImGui::InputInt("View Distance", &m_ViewSize, 2, 2))
		{
			if (m_ViewSize < 1) m_ViewSize = 1;
			updateTerrainGOs();
		}
		ImGui::Separator();
		regenerateTerrain |= m_BiomeGenerator->SettingsGUI();
	}
	ImGui::Separator();

	if (regenerateTerrain)
		regenerateAllHeightmaps();
}

void App1::regenerateAllHeightmaps()
{
	m_BiomeGenerator->UpdateBuffers(renderer->getDeviceContext());
	for (auto heightmap : m_Heightmaps)
		regenerateHeightmap(heightmap.second);
}

void App1::regenerateHeightmap(Heightmap* heightmap)
{
	if (m_HeightmapFilter)
		m_HeightmapFilter->Run(renderer->getDeviceContext(), heightmap, m_BiomeGenerator);
}

void App1::updateTerrainGOs()
{
	// work out which tile the player is located in
	XMFLOAT3 cameraPos = camera->getPosition();
	XMFLOAT2 worldTile = { floor(cameraPos.x / m_TileSize), floor(cameraPos.z / m_TileSize) };
	XMINT2 worldTileInt = { static_cast<int>(worldTile.x), static_cast<int>(worldTile.y) };

	// get all of the tiles within view of the player
	std::set<std::pair<int, int>> tilesInView;
	for (int i = 0; i < m_ViewSize; i++)
	for (int j = 0; j < m_ViewSize; j++)
	{
		XMINT2 tile{
			worldTileInt.x - (m_ViewSize / 2) + i,
			worldTileInt.y - (m_ViewSize / 2) + j,
		};

		// make sure this tile is in bounds
		if (tile.x >= 0 && tile.y >= 0)
			tilesInView.insert({ tile.x, tile.y });
	}

	// identify any terrains that are out of view
	std::queue<std::pair<int, int>> tilesToDelete;
	for (auto& heightmap : m_Heightmaps)
	{
		if (tilesInView.find(heightmap.first) == tilesInView.end())
			tilesToDelete.push(heightmap.first);
	}

	// identify any new terrains that need to be created
	std::queue<std::pair<int, int>> tilesToCreate;
	for (auto& tile : tilesInView)
	{
		if (m_Heightmaps.find(tile) == m_Heightmaps.end())
			tilesToCreate.push(tile);
	}

	// repurpose as many old terrains as possible
	// create new ones where required
	while (!tilesToCreate.empty())
	{
		auto& tile = tilesToCreate.front();

		if (tilesToDelete.empty())
		{
			XMFLOAT3 pos{ tile.first * m_TileSize, 0.0f, tile.second * m_TileSize };
			GameObject* newGO = new GameObject{ m_TerrainMesh, pos };
			m_GameObjects.push_back(newGO);
			m_Terrains.insert({ tile, m_GameObjects.back() });

			Heightmap* newHeightmap = new Heightmap(renderer->getDevice(), 1024);
			newHeightmap->SetOffset({
				static_cast<float>(tile.first),
				static_cast<float>(tile.second)
				});
			regenerateHeightmap(newHeightmap);

			m_Heightmaps.insert({ tile, newHeightmap });
		}
		else
		{
			auto& oldTile = tilesToDelete.front();

			// repurpose a terrain that is going to be deleted
			XMFLOAT3 pos{ tile.first * m_TileSize, 0.0f, tile.second * m_TileSize };

			GameObject* go = m_Terrains.at(oldTile);
			Heightmap* heightmap = m_Heightmaps.at(oldTile);

			go->transform.SetTranslation(pos);
			heightmap->SetOffset({ 
				static_cast<float>(tile.first),
				static_cast<float>(tile.second) 
				});
			regenerateHeightmap(heightmap);

			m_Terrains.erase(oldTile);
			m_Terrains.insert({ tile, go });

			m_Heightmaps.erase(oldTile);
			m_Heightmaps.insert({ tile, heightmap });

			tilesToDelete.pop();
		}

		tilesToCreate.pop();
	}

	// delete any left over terrains
	while (!tilesToDelete.empty())
	{
		// delete tile
		auto& tile = tilesToDelete.front();

		// delete heightmap
		delete m_Heightmaps.at(tile);
		m_Heightmaps.erase(tile);

		// delete terrain
		GameObject* goToDelete = m_Terrains.at(tile);
		for (auto it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
		{
			if (*it == goToDelete)
			{
				delete goToDelete;
				m_GameObjects.erase(it);
				break;
			}
		}
		m_Terrains.erase(tile);

		tilesToDelete.pop();
	}
}


void App1::saveSettings(const std::string& file)
{
	nlohmann::json serialized;

	serialized["biomeGenerator"] = m_BiomeGenerator->Serialize();

	// serialize light settings
	serialized["lightDir"] = SerializationHelper::SerializeFloat3(lightDir);
	serialized["lightDiffuse"] = SerializationHelper::SerializeFloat3(lightDiffuse);
	serialized["lightAmbient"] = SerializationHelper::SerializeFloat3(lightAmbient);
	serialized["lightSpecular"] = SerializationHelper::SerializeFloat3(lightSpecular);

	// camera
	serialized["cameraPos"] = SerializationHelper::SerializeFloat3(camera->getPosition());

	serialized["waterSettings"] = m_WaterShader->Serialize();

	std::string serializedString = serialized.dump();
	std::ofstream outfile(file);

	outfile << serialized << std::endl;
	outfile.close();
	
}

void App1::loadSettings(const std::string& file)
{
	// load data from file
	std::ifstream infile(file);
	nlohmann::json data;
	infile >> data;
	infile.close();

	if (data.contains("biomeGenerator")) m_BiomeGenerator->LoadFromJson(data["biomeGenerator"]);

	if (data.contains("lightDir")) SerializationHelper::LoadFloat3FromJson(&lightDir, data["lightDir"]);
	if (data.contains("lightDiffuse")) SerializationHelper::LoadFloat3FromJson(&lightDiffuse, data["lightDiffuse"]);
	if (data.contains("lightAmbient")) SerializationHelper::LoadFloat3FromJson(&lightAmbient, data["lightAmbient"]);
	if (data.contains("lightSpecular")) SerializationHelper::LoadFloat3FromJson(&lightSpecular, data["lightSpecular"]);
	
	light->setDiffuseColour(lightDiffuse.x, lightDiffuse.y, lightDiffuse.z, 1.0f);
	light->setAmbientColour(lightAmbient.x, lightAmbient.y, lightAmbient.z, 1.0f);
	light->setSpecularColour(lightSpecular.x, lightSpecular.y, lightSpecular.z, 1.0f);
	light->setDirection(lightDir.x, lightDir.y, lightDir.z);

	if (data.contains("cameraPos"))
	{
		XMFLOAT3 cameraPos;
		SerializationHelper::LoadFloat3FromJson(&cameraPos, data["cameraPos"]);
		camera->setPosition(cameraPos.x, cameraPos.y, cameraPos.z);
	}


	if (data.contains("waterSettings")) m_WaterShader->LoadFromJson(data["waterSettings"]);
}

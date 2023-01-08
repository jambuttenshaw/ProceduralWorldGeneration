#include "App1.h"

#include <nlohmann/json.hpp>

#include "HeightmapFilters.h"
#include "SerializationHelper.h"


App1::App1()
{
	m_TerrainMesh = nullptr;
	m_TerrainShader = nullptr;

	strcpy_s(m_SaveFilePath, "res/settings/earth.json");
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

	m_RenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_TerrainMesh = new TerrainMesh(renderer->getDevice());
	m_Cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext(), 2);

	camera->setPosition(-50.0f, 30.0f, -50.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	// create game objects
	createTerrain({ 0.0f, 0.0f, 0.0f });
	createTerrain({ 100.0f, 0.0f, 0.0f });
	createTerrain({ 0.0f, 0.0f, 100.0f });
	createTerrain({ 100.0f, 0.0f, 100.0f });

	// Initialise light
	light = new Light();
	light->setDiffuseColour(lightDiffuse.x, lightDiffuse.y, lightDiffuse.z, 1.0f);
	light->setAmbientColour(lightAmbient.x, lightAmbient.y, lightAmbient.z, 1.0f);
	light->setSpecularColour(lightSpecular.x, lightSpecular.y, lightSpecular.z, 1.0f);
	light->setDirection(lightDir.x, lightDir.y, lightDir.z);

	if (m_LoadOnOpen)
	{
		loadSettings(std::string(m_SaveFilePath));
		applyFilterStack();
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
		delete heightmap;
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
		XMMATRIX w = worldMatrix * go.transform.GetMatrix();
		switch (go.meshType)
		{
		case GameObject::MeshType::Regular:
			go.mesh.regular->sendData(renderer->getDeviceContext());
			m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, light);
			m_LightShader->render(renderer->getDeviceContext(), go.mesh.regular->getIndexCount());
			break;
		case GameObject::MeshType::Terrain:
			go.mesh.terrain->SendData(renderer->getDeviceContext());
			m_TerrainShader->SetShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_GOToHeightmap.at(goIndex)->GetSRV(), light);
			m_TerrainShader->Render(renderer->getDeviceContext(), go.mesh.terrain->GetIndexCount());
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
		m_WaterShader->setShaderParameters(renderer->getDeviceContext(), viewMatrix, projectionMatrix, m_RenderTarget->GetColourSRV(), m_RenderTarget->GetDepthSRV(), light, camera, m_Time);
		m_WaterShader->Render(renderer->getDeviceContext());
		renderer->setZBuffer(true);
	}
}

void App1::gui()
{
	bool regenerateTerrain = false;

	if (ImGui::CollapsingHeader("General"))
	{
		static bool showDemo = false;
		ImGui::Checkbox("Show demo", &showDemo);
		if (showDemo)
		{
			ImGui::ShowDemoWindow();
			return;
		}

		ImGui::Text("FPS: %.2f", timer->getFPS());
		ImGui::Checkbox("Wireframe mode", &wireframeToggle);
		ImGui::Separator();

		if (ImGui::TreeNode("Camera"))
		{
			XMFLOAT3 camPos = camera->getPosition();
			if (ImGui::DragFloat3("Camera Pos", &camPos.x, 0.1f))
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
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Graphics"))
	{
		if (ImGui::TreeNode("Ocean"))
		{
			m_WaterShader->SettingsGUI();

			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Terrain"))
		{
			m_TerrainShader->GUI();

			ImGui::TreePop();
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("GameObjects"))
	{
		int i = 0;
		for (auto& go : m_GameObjects)
		{
			if (ImGui::TreeNode((void*)(intptr_t)(i), "Game Object %d", i))
			{
				go.SettingsGUI();
				ImGui::Separator();

				ImGui::TreePop();
			}
			i++;
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Terrain Filters"))
	{
		int currentHeightmapFilter = 0;
		for (auto& name : m_AllFilterNames)
		{
			if (name == m_HeightmapFilter->Label())
				break;
			currentHeightmapFilter++;
		}
		if (ImGui::Combo("Filter", &currentHeightmapFilter, m_AllFilterNames.data(), m_AllFilterNames.size()))
		{
			delete m_HeightmapFilter;
			m_HeightmapFilter = createFilterFromIndex(currentHeightmapFilter);
		}

		if (m_HeightmapFilter)
		{
			ImGui::Separator();
			ImGui::Text("Filter Settings");
			ImGui::Separator();

			regenerateTerrain |= m_HeightmapFilter->SettingsGUI();
		}
	}
	ImGui::Separator();

	if (regenerateTerrain)
		applyFilterStack();
}

void App1::applyFilterStack()
{
	for (auto heightmap : m_Heightmaps)
	{
		m_HeightmapFilter->Run(renderer->getDeviceContext(), heightmap);
	}
}

void App1::createTerrain(const XMFLOAT3& pos)
{
	m_GameObjects.push_back({ m_TerrainMesh, pos });

	Heightmap* newHeightmap = new Heightmap(renderer->getDevice(), 1024);
	newHeightmap->SetOffset({ pos.x / m_TerrainMesh->GetSize(), pos.z / m_TerrainMesh->GetSize() });

	m_Heightmaps.push_back(newHeightmap);

	m_GOToHeightmap.insert({ m_GameObjects.size() - 1, newHeightmap });
}

IHeightmapFilter* App1::createFilterFromIndex(int index)
{
	IHeightmapFilter* newFilter = nullptr;
	switch (index)
	{
	case 0: newFilter = new SimpleNoiseFilter(renderer->getDevice()); break;
	case 1: newFilter = new RidgeNoiseFilter(renderer->getDevice()); break;
	case 2: newFilter = new WarpedSimpleNoiseFilter(renderer->getDevice()); break;
	case 3: newFilter = new TerrainNoiseFilter(renderer->getDevice()); break;
	default: break;
	}
	assert(newFilter != nullptr);

	return newFilter;
}

void App1::saveSettings(const std::string& file)
{
	nlohmann::json serialized;

	// serialize filter data
	serialized["filters"] = nlohmann::json::array();
	serialized["filters"].push_back(m_HeightmapFilter->Serialize());

	// serialize light settings
	serialized["lightDir"] = SerializationHelper::SerializeFloat3(lightDir);
	serialized["lightDiffuse"] = SerializationHelper::SerializeFloat3(lightDiffuse);
	serialized["lightAmbient"] = SerializationHelper::SerializeFloat3(lightAmbient);
	serialized["lightSpecular"] = SerializationHelper::SerializeFloat3(lightSpecular);

	serialized["waterSettings"] = m_WaterShader->Serialize();
	serialized["terrainSettings"] = m_TerrainShader->Serialize();

	std::string serializedString = serialized.dump();
	std::ofstream outfile(file);

	outfile << serialized << std::endl;
	outfile.close();
	
}

void App1::loadSettings(const std::string& file)
{
	
	// clear out existing settings
	delete m_HeightmapFilter;

	// load data from file
	std::ifstream infile(file);
	nlohmann::json data;
	infile >> data;
	infile.close();

	// construct objects from data
	if (data.contains("filters"))
	{
		for (auto filter : data["filters"])
		{
			if (!filter.contains("name")) continue;
			// work out the filter index
			int index = 0;
			for (const auto& name : m_AllFilterNames)
			{
				if (name == filter["name"]) break;
				index++;
			}
			if (index == m_AllFilterNames.size()) continue; // saved filter name must be invalid

			m_HeightmapFilter = createFilterFromIndex(index);
			m_HeightmapFilter->LoadFromJson(filter);
		}
	}

	if (data.contains("lightDir")) SerializationHelper::LoadFloat3FromJson(&lightDir, data["lightDir"]);
	if (data.contains("lightDiffuse")) SerializationHelper::LoadFloat3FromJson(&lightDiffuse, data["lightDiffuse"]);
	if (data.contains("lightAmbient")) SerializationHelper::LoadFloat3FromJson(&lightAmbient, data["lightAmbient"]);
	if (data.contains("lightSpecular")) SerializationHelper::LoadFloat3FromJson(&lightSpecular, data["lightSpecular"]);
	
	light->setDiffuseColour(lightDiffuse.x, lightDiffuse.y, lightDiffuse.z, 1.0f);
	light->setAmbientColour(lightAmbient.x, lightAmbient.y, lightAmbient.z, 1.0f);
	light->setSpecularColour(lightSpecular.x, lightSpecular.y, lightSpecular.z, 1.0f);
	light->setDirection(lightDir.x, lightDir.y, lightDir.z);

	if (data.contains("waterSettings")) m_WaterShader->LoadFromJson(data["waterSettings"]);
	if (data.contains("terrainSettings")) m_TerrainShader->LoadFromJson(data["terrainSettings"]);
}

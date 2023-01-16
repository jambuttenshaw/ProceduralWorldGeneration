#include "App1.h"

#include <nlohmann/json.hpp>

#include "HeightmapFilter.h"
#include "SerializationHelper.h"

#include "BiomeGenerator.h"


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
	m_BiomeMapShader = new BiomeMapShader(renderer->getDevice(), hwnd);

	m_RenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_TerrainMesh = new TerrainMesh(renderer->getDevice());
	m_Cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext(), 2);
	m_OrthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 200, 200, (screenWidth / 2) - 100, (screenHeight / 2) - 100);

	camera->setPosition(-50.0f, 30.0f, -50.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	// create game objects
	createTerrain({ 0.0f, 0.0f, 0.0f });
	createTerrain({ 100.0f, 0.0f, 0.0f });
	createTerrain({ 0.0f, 0.0f, 100.0f });
	createTerrain({ 100.0f, 0.0f, 100.0f });
	createTerrain({ 200.0f, 0.0f, 000.0f });
	createTerrain({ 200.0f, 0.0f, 100.0f });
	createTerrain({ 200.0f, 0.0f, 200.0f });
	createTerrain({ 100.0f, 0.0f, 200.0f });
	createTerrain({ 000.0f, 0.0f, 200.0f });

	m_WorldMinPos = { 0, 0 };
	m_WorldSize = { 3, 3 };

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

	renderer->setZBuffer(false);
	if (m_BiomeGenerator->ShowBiomeMap())
	{
		m_OrthoMesh->sendData(renderer->getDeviceContext());

		XMMATRIX worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();

		m_BiomeMapShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_BiomeGenerator, m_WorldMinPos, m_WorldSize);
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
			m_TerrainShader->SetShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_GOToHeightmap.at(goIndex), m_BiomeGenerator, light);
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
		m_WaterShader->setShaderParameters(renderer->getDeviceContext(), viewMatrix, projectionMatrix, m_RenderTarget->GetColourSRV(), m_RenderTarget->GetDepthSRV(), light, camera, m_Time, m_BiomeGenerator);
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

	if (ImGui::CollapsingHeader("Ocean"))
	{
		m_WaterShader->SettingsGUI();
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

	if (ImGui::CollapsingHeader("Terrain"))
	{
		regenerateTerrain |= m_BiomeGenerator->SettingsGUI();
	}
	ImGui::Separator();

	if (regenerateTerrain)
		applyFilterStack();
}

void App1::applyFilterStack()
{
	m_BiomeGenerator->UpdateBuffers(renderer->getDeviceContext());
	for (auto heightmap : m_Heightmaps)
	{
		if (m_HeightmapFilter)
			m_HeightmapFilter->Run(renderer->getDeviceContext(), heightmap, m_BiomeGenerator);
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


void App1::saveSettings(const std::string& file)
{
	nlohmann::json serialized;

	serialized["biomeGenerator"] = m_BiomeGenerator->Serialize();

	// serialize light settings
	serialized["lightDir"] = SerializationHelper::SerializeFloat3(lightDir);
	serialized["lightDiffuse"] = SerializationHelper::SerializeFloat3(lightDiffuse);
	serialized["lightAmbient"] = SerializationHelper::SerializeFloat3(lightAmbient);
	serialized["lightSpecular"] = SerializationHelper::SerializeFloat3(lightSpecular);

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

	if (data.contains("waterSettings")) m_WaterShader->LoadFromJson(data["waterSettings"]);
}

#include "App.h"
#include <vector>

App::App() :
	m_TerrainResolution{ 128 },
	m_Light{ std::make_unique<Light>() }
{
	m_InstanceShader = nullptr;
}

void App::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"block", L"res/BlockTex.png");
	textureMgr->loadTexture(L"grass", L"res/grass.png");

	// Create Mesh object and shader object
	m_Terrain			= std::make_unique<TerrainMesh>(renderer->getDevice(), renderer->getDeviceContext());
	m_LightShader		= std::make_unique<LightShader>(renderer->getDevice(), hwnd);

	m_InstancedCube		= std::make_unique<InstancedCubeMesh>(renderer->getDevice(), renderer->getDeviceContext(), 1);
	m_InstanceShader	= std::make_unique<InstanceShader>(renderer->getDevice(), hwnd);
		
	BuildCubeInstances();

	//Make unlit shader for rendering the Quad
	m_UnlitShader		= std::make_unique<UnlitShader>(renderer->getDevice(), hwnd);

	//Make a 100x100 quad and move it into view
	m_Quad = std::make_unique<QuadMeshT>(renderer->getDevice(), 50, 50);
	m_Quad->m_Transform = XMMatrixTranslation(-50, 0, 50);

	//Initialise writable texture
	constexpr int texSize = 1024;
	m_Texture = std::make_unique<WritableTexture>(texSize, texSize, renderer->getDevice());

	//An example of how SetPixel can be used to set the value of an individual pixel
	for (int y = 0; y < texSize; y++) {
		for (int x = 0; x < texSize; x++) {
			float c = (sin(x/16.0f)+1)*0.5f;
			m_Texture->SetPixel(x, y, c, c, c, 1.0f);
		}
	}
	//Make all of your changes, then call Update to update the texture resource
	m_Texture->Update(renderer->getDeviceContext());


	// Initialise light
	m_Light = std::make_unique<Light>();
	m_Light->setDiffuseColour(0.8f, 0.8f, 0.8f, 1.0f);
	m_Light->setAmbientColour(0.1f, 0.1f,0.1f, 1.0f);
	m_Light->setDirection(-0.578f, -0.578f, 0.1f);

	//Line
	m_Line = std::make_unique<LineMesh>(renderer->getDevice(), renderer->getDeviceContext());
	m_Line->AddLine(XMVECTOR{0,0,0,0}, XMVECTOR{ -10,0,-10,0 });

	m_Line->BuildLine(renderer->getDeviceContext(), renderer->getDevice());

	//Initialise Camera Position
	camera->setPosition(-16, 60, -15);
	camera->setRotation(25, 45, 0);
}


App::~App()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
}


bool App::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}
void App::BuildCubeInstances() {
	constexpr int width = 64;
	constexpr int maxCubes = width * width * width;

	vector<XMFLOAT3> positions;
	vector<XMFLOAT2> uvs;

	int instanceCount = 0;

	//Loop through every possible point within our voxel space and see if it should be added to our instance list
	for (int i = 0; i < maxCubes; i++) {
		const int x = i % width;
		const int y = ((i / width) % width);
		const int z = (i / (width * width));

		//Create two crossing sine waves
		const float y1 = (sin((float)(x) / 8.0f) + 1.0f) * 16.0f;
		const float y2 = (sin((float)(z) / 4.0f) + 1.0f) * 16.0f;

		//If the current voxel is below both sin waves
		if (y < y1 && y < y2) {
			//Add it to our list voxels to draw
			positions.push_back( XMFLOAT3(2.0f * x, 2.0f * y, 2.0f * z));

			//Add a UV offset so we can draw different textures
			if ((y+1 < y1 && y+1 < y2)) {
				//Is not on top				
				uvs.push_back(XMFLOAT2{ 0.5,0.0 });
			}
			else {
				//If it's high up, use the snow section of the texture
				if (y > 20)
					uvs.push_back(XMFLOAT2{ 0.0,0.5 });
				else
					uvs.push_back(XMFLOAT2{ 0.0,0.0 });
			}
			instanceCount++;
		}
	}

	m_InstancedCube->initBuffers(renderer->getDevice(), positions.data(), uvs.data(), instanceCount);
}

bool App::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	m_InstanceShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"block"), m_Light.get());
	m_InstancedCube->sendDataInstanced(renderer->getDeviceContext());
	m_InstanceShader->renderInstanced(renderer->getDeviceContext(), m_InstancedCube->getIndexCount(), m_InstancedCube->GetInstanceCount());
	
	//we can multiply the world transform by the object's transform to move it into position
	m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * m_Quad->m_Transform, viewMatrix, projectionMatrix, m_Texture->GetTextureView());
	m_Quad->sendData(renderer->getDeviceContext());
	m_UnlitShader->render(renderer->getDeviceContext(), m_Quad->getIndexCount());

	//Render line
	if (m_Line->getIndexCount() > 0) {
		m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"));
		m_Line->sendData(renderer->getDeviceContext());
		m_UnlitShader->render(renderer->getDeviceContext(), m_Line->getIndexCount());
	}

	//Render terrain
	m_Terrain->sendData(renderer->getDeviceContext());
	m_LightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * XMMatrixTranslation(0,0,-100), viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), m_Light.get());
	m_LightShader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());


	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::SliderInt("Terrain Resolution", &m_TerrainResolution, 2, 1024);
	if (ImGui::Button("Regenerate Terrain")) {
		if (m_TerrainResolution != m_Terrain->GetResolution()) {
			m_Terrain->Resize(m_TerrainResolution);
		}
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	ImGui::Text("Rendering %i cubes", m_InstancedCube->GetInstanceCount());
	
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
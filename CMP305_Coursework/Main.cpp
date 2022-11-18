// Main.cpp
#include "System.h"
#include "App.h"
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App* app = new App();
	std::unique_ptr<System> system = std::make_unique<System>(app, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	return 0;
}
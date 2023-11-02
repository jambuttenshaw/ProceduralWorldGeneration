// Main.cpp
#include "System.h"
#include "App1.h"
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App1* app = new App1();
	std::unique_ptr<System> system = std::make_unique<System>(app, 2560, 1440, true, true);

	// Initialize and run the system object.
	system->run();

	return 0;
}
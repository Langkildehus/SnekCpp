#include <thread>
#include <d3d9.h>

#include "gui.h"
#include "grid.h"

int __stdcall wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// Create grid
	Grid grid = Grid(40, 30, 30);

	// Create GUI
	gui::Init("TEST MENU", "TEST MENU CLASS");

	while (gui::exit)
	{
		gui::BeginRender();

		grid.Render();

		//gui::Render(); // ONLY DEMO
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Destroy GUI
	gui::Destroy();

	return 0;
}

#include <thread>
#include <vector>
#include <unordered_map>

// Networking imports
#include "..\Networking\networking.h"
#include "common.h"
#include "snakeclient.h"

// Library imports
#include <d3d9.h>
#include "imgui\imgui.h"

// Headerfiles
#include "gui.h"
#include "grid.h"
#include "player.h"
#include "powerups.h"

#define FRAMETIME 0.125f;

int __stdcall wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// Start creating game
	std::unordered_map<uint32_t, Player> players;
	std::vector<Powerup> powerups;

	// Create grid
	Grid grid = Grid(40, 30, 30, players, powerups);

	// Frame timer
	float nextFrame = 0.0f;




	// TESTING VARS
	players[0] = Player(34, 5, &grid);
	players[0].SetDirection(Direction::Left);

	ImU32 color = ImColor(1.0f, 0.1f, 0.1f, 1.0f);
	powerups.push_back(Powerup(5, 5, color));






	// Create client instance and connect to server
	SnakeClient client;
	client.Connect(IP, PORT);

	while (!client.IsConnected())
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	// Create GUI
	gui::Init("Snake Battle Royale", "Snake Battle Royale Class");

	while (gui::exit)
	{

		/* --------------- GAME LOOP --------------- */
		client.HandleMessages();

		ImGuiIO& io = ImGui::GetIO();
		nextFrame -= io.DeltaTime;

		if (nextFrame <= 0)
		{
			nextFrame = FRAMETIME;

			// Update all players
			for (std::pair<const uint32_t, Player>& player : players)
			{
				player.second.Move();

				for (unsigned int c = 0; c < powerups.size(); c++)
				{
					if (powerups[c].CheckCollision(player.second.GetHead()))
					{
						powerups.erase(powerups.begin() + c);
						player.second.AddLength(1);
						break;
					}
				}
			}
		}
		

		// Update screen
		gui::BeginRender();
		grid.Render();
		//gui::Render(); // RENDERS DEMO GUI
		gui::EndRender();
	}

	// Destroy GUI
	gui::Destroy();

	// Disconnect from server
	client.Disconnect();

	return 0;
}

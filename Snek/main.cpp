#include <thread>
#include <vector>
#include <unordered_map>

// Library imports
#include <GLFW\glfw3.h>

// Networking imports
#include "..\Networking\networking.h"
#include "common.h"

// Headerfiles
#include "gui.h"
#include "snakeclient.h"
#include "grid.h"
#include "player.h"
#include "powerups.h"

#define FRAMETIME 0.125f;
#define WIDTH 1300
#define HEIGHT 1030

int main()
{
	bool mainLoop = true;
	int clientID = -1;

	// Start creating local game
	std::unordered_map<uint32_t, Player> players;
	std::vector<Powerup> powerups;

	// Create grid
	Grid grid = Grid(GRIDWIDTH, GRIDHEIGHT, 30, &mainLoop, WIDTH, HEIGHT, players, powerups);

	// Frame timer
	float nextFrame = 0.0f;




	// TESTING VARS
	/*
	players[0] = Player(34, 5, &grid);
	players[0].SetDirection(Direction::Left);

	ImU32 color = ImColor(1.0f, 0.1f, 0.1f, 1.0f);
	powerups.push_back(Powerup(5, 5, color));
	*/



	// Create GUI
	Gui gui;
	GLFWwindow* window = gui.Init(WIDTH, HEIGHT, "Snake Battle Royale");
	if (!window)
		return 1;

	// Create client instance and connect to server
	SnakeClient client = SnakeClient(clientID);
	client.Connect(IP, PORT);

	while (!client.IsConnected())
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	while (clientID < 0 && !glfwWindowShouldClose(window))
	{
		/* --------- WAITING FOR CONNECTION -------- */
		glfwPollEvents();
		client.HandleMessages();

		// Update screen
		gui.NewFrame();
		// THIS IS WHERE DRAWING SHOULD BE HAPPENING
		grid.Render();

		gui.Render();
		glfwSwapBuffers(window);
	}

	while (mainLoop && !glfwWindowShouldClose(window))
	{
		/* --------------- GAME LOOP --------------- */
		glfwPollEvents();

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
		gui.NewFrame();
		// THIS IS WHERE DRAWING SHOULD BE HAPPENING
		grid.Render();

		gui.Render();
		glfwSwapBuffers(window);
	}

	// Destroy GUI
	gui.Shutdown();

	// Disconnect from server
	client.Disconnect();

	return 0;
}

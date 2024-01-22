#include "pch.h"

// Headerfiles
#include "common.h"
#include "gui.h"
#include "snakeclient.h"
#include "grid.h"
#include "player.h"
#include "powerups.h"

#define FRAMETIME 0.15f
#define WIDTH 1300
#define HEIGHT 1030

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

SnakeClient* client;
int clientID = -1;
std::unordered_map<uint32_t, Player> players;

int main()
{
	bool mainLoop = true;

	// Start creating local game
	std::vector<Powerup> powerups;

	// Create grid
	Grid grid = Grid(GRIDWIDTH, GRIDHEIGHT, 30, &mainLoop, WIDTH, HEIGHT, players, powerups);

	// Frame timer
	float nextFrame = 0.0f;

	// Create GUI
	Gui gui;
	GLFWwindow* window = gui.Init(WIDTH, HEIGHT, "Snake Battle Royale");
	if (!window)
	{
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);

	// Create client instance and connect to server
	client = new SnakeClient(clientID, powerups, players, &grid);

	if (!client)
	{
		std::cout << "Error instantiating client\n";
		gui.Shutdown();
		return -2;
	}

	client->Connect(IP, PORT);

	while (clientID < 0 && !glfwWindowShouldClose(window))
	{
		gui.NewFrame();
		gui.Render();
		glfwSwapBuffers(window);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		if (client->IsConnected())
		{
			client->HandleMessages();
		}
	}

	if (!glfwWindowShouldClose(window))
	{
		std::cout << "CLIENTID RECEIVED: " << clientID << "\n";
	}

	while (mainLoop && !glfwWindowShouldClose(window))
	{
		/* --------------- GAME LOOP --------------- */
		glfwPollEvents();

		ImGuiIO& io = ImGui::GetIO();
		nextFrame -= io.DeltaTime;

		if (nextFrame <= 0)
		{
			nextFrame += FRAMETIME;

			// Update all players
			if (client->IsGameStarted())
			{
				for (std::pair<const uint32_t, Player>& player : players)
				{
					player.second.Move();
				}
			}

			// Check collission for powerups
			for (unsigned int c = 0; c < powerups.size(); c++)
			{
				if (powerups[c].CheckCollision(players[clientID].GetHead()))
				{
					client->EatPowerup(c);
					break;
				}
			}

			client->HandleMessages();

			// Update position to server
			client->UpdateServer();
		}

		// Update screen
		gui.NewFrame();

		/* DRAW FRAME HERE */

		grid.Render();

		/* DRAW FRAME HERE */

		gui.Render();
		glfwSwapBuffers(window);
	}

	// Destroy GUI
	gui.Shutdown();

	// Disconnect from server
	client->Disconnect();
	delete client;

	return 0;
}

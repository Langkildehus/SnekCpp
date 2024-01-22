#include "pch.h"

#include "snakeclient.h"

extern SnakeClient* client;
extern int clientID;
extern std::unordered_map<uint32_t, Player> players;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Ignore inputs until connection to server has been established
	if (clientID < 0)
	{
		return;
	}

	// Press P to start game
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		client->AttemptGameStart();
	}

	// Movement
	if ((key == GLFW_KEY_W || key == GLFW_KEY_UP) && action == GLFW_PRESS && players[clientID].GetDirection() != Direction::Down)
	{
		players[clientID].SetDirection(Direction::Up);
	}
	if ((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && action == GLFW_PRESS && players[clientID].GetDirection() != Direction::Right)
	{
		players[clientID].SetDirection(Direction::Left);
	}
	if ((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && action == GLFW_PRESS && players[clientID].GetDirection() != Direction::Up)
	{
		players[clientID].SetDirection(Direction::Down);
	}
	if ((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && action == GLFW_PRESS && players[clientID].GetDirection() != Direction::Left)
	{
		players[clientID].SetDirection(Direction::Right);
	}
}

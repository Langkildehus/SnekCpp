#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

//#include <imgui.h>

#include "..\Snek\common.h"
#include "..\Snek\player.h"
#include "..\Snek\powerups.h"

#include "server.h"

Server::Server(int port) : net::ServerInterface<MessageTypes>(port)
{
	GenerateGrid();
	GenerateSpawnpoints();
}

bool Server::OnClientConnect(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	// Allow all clients if spawnpoints are available
	std::cout << "New connection received from: " << client->GetIP() << "\n";

	if (spawnpoints.size() > 0)
		return true;

	std::cout << "No more spawnpoints available!\n";
	return false;
}

void Server::OnClientValidated(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	// Find spawnposition for player
	Position spawnpoint = spawnpoints.back();
	spawnpoints.pop_back();

	// Create player locally
	PlayerData player;
	player.direction = Direction::Up;
	player.color = ImColor(1.0f, 0.1f, 0.1f, 1.0f);
	player.tail.emplace_front(spawnpoint.x, spawnpoint.y);

	players[client->GetID()] = player;

	// Send game state to new client
	net::Message<MessageTypes> msgState;
	msgState.header.id = MessageTypes::ServerValidation;

	// Add players to msg
	for (std::pair<const uint32_t, PlayerData>& otherPlayer : players)
	{
		AddPlayerToMsg(msgState, otherPlayer.second);

		// Add ID to top of player stack
		msgState << otherPlayer.first;
	}
	msgState << players.size();

	// Add powerups to msg
	for (PowerupData powerup : powerups)
		msgState << powerup;
	msgState << powerups.size();

	// Push ID to the top of the stack
	msgState << client->GetID();
	client->Send(msgState);


	// Update all other clients
	std::cout << "[" << client->GetID() << "]: Sending client connection to other clients\n";

	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::NewPlayer;

	AddPlayerToMsg(msg, player);

	msg << client->GetID();

	MessageAllClients(msg);

	if (players.size() % 2 != 0)
		GeneratePowerup();
}

void Server::OnClientDisconnect(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	// Send some sort of RemovePlayer msg to all clients
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::RemovePlayer;
	msg << client->GetID();
	MessageAllClients(msg, client);

	// Regenerate the lost spawnpoint

	// Delete player locally
	players.erase(client->GetID());

	// HANDLE COLORS FOR PLAYERS
	// SEND GAMESTATE OF PLAYERS&POWERUPS ON CONNECTION
}

void Server::OnMessage(std::shared_ptr<net::Connection<MessageTypes>> client, net::Message<MessageTypes>& msg)
{
	switch (msg.header.id)
	{
	case MessageTypes::PowerupEaten:
	{
		std::cout << "[" << client->GetID() << "]: Ate powerup\n";

		// Get powerup that was eaten
		PowerupData eatenPowerup;
		msg >> eatenPowerup;

		DeletePowerup(eatenPowerup);

		GeneratePowerup();
		break;
	}

	case MessageTypes::UpdatePlayer:
	{
		std::cout << "[" << client->GetID() << "]: Update player position\n";

		// Generate msg to other clients
		net::Message<MessageTypes> updateMsg;
		updateMsg.header.id = MessageTypes::UpdatePlayer;

		// Delete tail locally
		PlayerData& player = players.at(client->GetID());
		player.tail.clear();

		// Get tail length
		uint32_t length;
		msg >> length;

		// Update tail locally
		for (uint32_t c = 0; c < length; c++)
		{
			Position pos;
			msg >> pos;
			player.tail.push_back(pos);
		}

		// Update tale to other clients
		AddPlayerToMsg(updateMsg, player);
		updateMsg << client->GetID();
		MessageAllClients(updateMsg, client);

		break;
	}
	}
}

void Server::DeletePowerup(PowerupData powerup, std::shared_ptr<net::Connection<MessageTypes>> ignoreClient)
{
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::PowerupEaten;
	msg << powerup;

	// Delete powerup locally
	for (int c = 0; c < powerups.size(); c++)
	{
		if (powerups[c].x == powerup.x && powerups[c].y == powerup.y)
		{
			powerups.erase(powerups.begin() + c);
			break;
		}
	}

	// Tell all clients to delete powerup
	MessageAllClients(msg, ignoreClient); // PROBLEM WITH IGNORECLIENT
}

void Server::GeneratePowerup()
{
	// Generate new powerup position
	std::vector<Position> freeGrid(GRIDWIDTH * GRIDHEIGHT);
	GetFreeGrid(freeGrid);
	Position spawnPos = freeGrid[rand() % freeGrid.size()];

	// Generate powerup
	PowerupData powerup{ spawnPos.x, spawnPos.y };
	powerups.push_back(powerup);

	// Send powerup to clients
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::NewPowerup;
	msg << powerup;

	MessageAllClients(msg);
}

void Server::GetFreeGrid(std::vector<Position>& freeGrid)
{
	int freeGridArray[GRIDWIDTH][GRIDHEIGHT] = { 1 };

	for (std::pair<const uint32_t, PlayerData>& player : players)
	{
		for (Position pos : player.second.tail)
			freeGridArray[pos.x][pos.y] = 0;
	}

	for (PowerupData& powerup : powerups)
	{
		freeGridArray[powerup.x][powerup.y] = 0;
	}

	for (int x = 0; x < GRIDWIDTH; x++)
	{
		for (int y = 0; y < GRIDHEIGHT; y++)
		{
			if (freeGridArray[x][y] == 1)
				freeGrid.emplace_back(x, y);
		}
	}
}

void Server::GenerateGrid()
{
	for (int x = 0; x < GRIDWIDTH; x++)
	{
		for (int y = 0; y < GRIDHEIGHT; y++)
			grid.emplace_back(x, y);
	}
}

void Server::GenerateSpawnpoints()
{
	spawnpoints = std::vector<Position>(6);
	spawnpoints.emplace_back(Position(GRIDWIDTH / 4,		GRIDHEIGHT / 3));
	spawnpoints.emplace_back(Position(2 * GRIDWIDTH / 4,	GRIDHEIGHT / 3));
	spawnpoints.emplace_back(Position(3 * GRIDWIDTH / 4,	GRIDHEIGHT / 3));
	spawnpoints.emplace_back(Position(GRIDWIDTH / 4,		2 * GRIDHEIGHT / 3));
	spawnpoints.emplace_back(Position(2 * GRIDWIDTH / 4,	2 * GRIDHEIGHT / 3));
	spawnpoints.emplace_back(Position(3 * GRIDWIDTH / 4,	2 * GRIDHEIGHT / 3));
}

void Server::AddPlayerToMsg(net::Message<MessageTypes>& msg, PlayerData& player)
{
	for (Position pos : player.tail)
		msg << pos;

	// Add tail size to top of stack
	msg << player.tail.size();
}

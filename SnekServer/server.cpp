#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

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
	{
		return true;
	}

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
	std::cout << "Players size: " << players.size() << "\n";

	// Add powerups to msg
	for (PowerupData powerup : powerups)
	{
		msgState << powerup;
	}
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

	MessageAllClients(msg, client);

	if (players.size() % 2 != 0)
	{
		GeneratePowerup();
	}
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
	std::cout << "MSG RECEIVED: " << static_cast<std::underlying_type<MessageTypes>::type>(msg.header.id) << "\n";

	switch (msg.header.id)
	{
	case MessageTypes::PowerupEaten:
	{
		std::cout << "[" << client->GetID() << "]: Ate powerup\n";

		// Get powerup that was eaten
		PowerupData eatenPowerup;
		msg >> eatenPowerup;

		DeletePowerup(eatenPowerup, client);

		GeneratePowerup();
		break;
	}

	case MessageTypes::UpdatePlayer:
	{
		std::cout << "[" << client->GetID() << "]: Update player position\n";

		// Update all other clients
		MessageAllClients(msg, client);

		int _clientID;
		msg >> _clientID;

		std::cout << "Received clientID: " << _clientID << "\n";

		// Delete tail locally
		PlayerData& player = players.at(client->GetID());
		player.tail.clear();

		// Get tail length
		size_t tailLength;
		msg >> tailLength;

		std::cout << "TailLength: " << tailLength << "\n";

		// Update tail locally
		for (unsigned int c = 0; c < 1; c++)//tailLength; c++)
		{
			Position pos;
			msg >> pos;

			std::cout << "Tail size after clear(): " << player.tail.size() << "\n";
			std::cout << "x: " << pos.x << ", y: " << pos.y << "\n";
			player.tail.emplace_front(pos.x, pos.y);
		}
		break;
	}

	default:
	{
		std::cout << "----------------------------------------------------------\n";
		std::cout << "Msg type:" << (uint32_t)msg.header.id << "\n";
		std::cout << "Unhandled msg received!\n";

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
	std::vector<Position> freeGrid;
	GetFreeGrid(freeGrid);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(0, (int)freeGrid.size() - 1);

	// Generate new powerup position
	Position spawnPos = freeGrid[distribution(gen)];

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
	int freeGridArray[GRIDWIDTH][GRIDHEIGHT] = { };

	for (std::pair<const uint32_t, PlayerData>& player : players)
	{
		for (Position pos : player.second.tail)
		{
			freeGridArray[pos.x][pos.y] = 1;
		}
	}

	for (PowerupData& powerup : powerups)
	{
		freeGridArray[powerup.x][powerup.y] = 1;
	}

	for (int x = 0; x < GRIDWIDTH; x++)
	{
		for (int y = 0; y < GRIDHEIGHT; y++)
		{
			if (freeGridArray[x][y] == 0)
			{
				freeGrid.emplace_back(x, y);
			}
		}
	}
}

void Server::GenerateGrid()
{
	for (int x = 0; x < GRIDWIDTH; x++)
	{
		for (int y = 0; y < GRIDHEIGHT; y++)
		{
			grid.emplace_back(x, y);
		}
	}
}

void Server::GenerateSpawnpoints()
{
	spawnpoints = std::vector<Position>(6);
	spawnpoints.emplace_back(GRIDWIDTH / 4,		GRIDHEIGHT / 3);
	spawnpoints.emplace_back(2 * GRIDWIDTH / 4,	GRIDHEIGHT / 3);
	spawnpoints.emplace_back(3 * GRIDWIDTH / 4,	GRIDHEIGHT / 3);
	spawnpoints.emplace_back(GRIDWIDTH / 4,		2 * GRIDHEIGHT / 3);
	spawnpoints.emplace_back(2 * GRIDWIDTH / 4,	2 * GRIDHEIGHT / 3);
	spawnpoints.emplace_back(3 * GRIDWIDTH / 4,	2 * GRIDHEIGHT / 3);
}

void Server::AddPlayerToMsg(net::Message<MessageTypes>& msg, PlayerData& player)
{
	for (Position pos : player.tail)
	{
		msg << pos;
	}

	// Add tail size to top of stack
	msg << player.tail.size();
}

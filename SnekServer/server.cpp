#include "pch.h"

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
		return !gameStarted;
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
	player.color = ImColor(rand() % 256, rand() % 256, rand() % 256, 255);
	player.tail.emplace_front(spawnpoint.x, spawnpoint.y);

	players[client->GetID()] = player;

	// Send game state to new client
	net::Message<MessageTypes> msgState;
	msgState.header.id = MessageTypes::ServerValidation;

	// Add players to msg
	for (std::pair<const uint32_t, PlayerData>& otherPlayer : players)
	{
		AddPlayerToMsg(msgState, otherPlayer.second);

		msgState << otherPlayer.second.color;

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
	msg << player.color;

	msg << client->GetID();

	MessageAllClients(msg, client);

	int powerupCount = players.size() % 2 * players.size() / 2 + 1;
	while (powerups.size() < powerupCount)
	{
		GeneratePowerup();
	}
}

void Server::OnClientDisconnect(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	int id = client->GetID();
	std::cout << "Client disconnected: " << id << "\n";

	// Send RemovePlayer msg to all clients
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::RemovePlayer;
	msg << id;
	MessageAllClients(msg, client);

	// Regenerate the lost spawnpoint

	// Delete player locally
	players.erase(id);

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

		DeletePowerup(eatenPowerup, client);

		GeneratePowerup();
		break;
	}

	case MessageTypes::UpdatePlayer:
	{
		// Update all other clients
		MessageAllClients(msg, client);

		int clientID;
		msg >> clientID;

		// Delete tail locally
		PlayerData& player = players.at(client->GetID());
		player.tail.clear();

		msg >> player.direction;

		// Get tail length
		size_t tailLength;
		msg >> tailLength;

		// Update tail locally
		for (unsigned int c = 0; c < tailLength; c++)//tailLength; c++)
		{
			Position pos;
			msg >> pos;

			player.tail.emplace_front(pos.x, pos.y);
		}

		PlayerCollision(clientID);
		break;
	}

	case MessageTypes::StartGame:
	{
		std::cout << "Game Started by" << client->GetID() << "\n";
		gameStarted = true;
		MessageAllClients(msg);
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

void Server::SendKillMessage(int clientID)
{
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::KillPlayer;
	msg << clientID;

	MessageAllClients(msg);
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

void Server::PlayerCollision(int clientID) // Hit/Kill detection
{
	for (std::pair<const uint32_t, PlayerData>& player : players) // For all players
	{
		if (player.first == clientID) // Check for clients own tail
		{
			for (int c = 4; c < player.second.tail.size(); c++)
			{
				if (player.second.tail[c].x == players[clientID].tail[0].x && player.second.tail[c].y == players[clientID].tail[0].y)
				{
					SendKillMessage(clientID);
					return;
				}
			}
		}
		else // Check for other players tails
		{
			for (Position& pos : player.second.tail)
			{
				if (pos.x == players[clientID].tail[0].x && pos.y == players[clientID].tail[0].y)
				{
					SendKillMessage(clientID);
					return;
				}
			}
		}
	}
}

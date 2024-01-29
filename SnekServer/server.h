#pragma once

#include <vector>
#include <unordered_map>

#include "..\Networking\networking.h"
#include "..\Snek\common.h"
#include "..\Snek\player.h"
#include "..\Snek\powerups.h"

class Server : public net::ServerInterface<MessageTypes>
{
public:
	// Server constructor
	Server(int port);

protected:
	// Called when a new client creates a connection
	// Returning true allows connection, returning false ignores connection
	bool OnClientConnect(std::shared_ptr<net::Connection<MessageTypes>> client) override;

	// Called when a client is validated (connection is verified)
	void OnClientValidated(std::shared_ptr<net::Connection<MessageTypes>> client) override;

	// Called when a client disconnects from the game
	void OnClientDisconnect(std::shared_ptr<net::Connection<MessageTypes>> client) override;

	// Called when server receives a msg from any client
	void OnMessage(std::shared_ptr<net::Connection<MessageTypes>> client, net::Message<MessageTypes>& msg) override;

	// Get free grid positions
	void GetFreeGrid(std::vector<Position>& freeGrid);

	void DeletePowerup(PowerupData powerup, std::shared_ptr<net::Connection<MessageTypes>> ignoreClient = nullptr);
	void GeneratePowerup();

	void GenerateGrid();
	void GenerateSpawnpoints();

	void SendKillMessage(int clientID);
	void AddPlayerToMsg(net::Message<MessageTypes>& msg, PlayerData& player);
	void PlayerCollision(int clientID);

private:
	std::unordered_map<uint32_t, PlayerData> players;
	std::vector<PowerupData> powerups;

	std::vector<Position> grid;

	std::vector<Position> spawnpoints;

	bool gameStarted = false;
};

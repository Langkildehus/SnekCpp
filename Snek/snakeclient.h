#pragma once

#include "common.h"
#include "powerups.h"
#include "grid.h"

class SnakeClient : public net::ClientInterface<MessageTypes>
{
public:
	SnakeClient(int& _clientID, std::vector<Powerup>& _powerups, std::unordered_map<uint32_t, Player>& _players, Grid* grid);

	// Should be called to send update information to server
	void UpdateServer();

	// Should be called every frame to handle incoming messages
	void HandleMessages();

	void EatPowerup(unsigned int c);

	// Attempts to start the game
	void AttemptGameStart();

	bool IsGameStarted() const;

private:
	bool gameStarted = false;

	int& clientID;
	std::vector<Powerup>& powerups;
	std::unordered_map<uint32_t, Player>& players;
	Grid* gridPointer;
};

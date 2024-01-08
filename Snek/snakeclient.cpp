#include <vector>
#include <iostream>

#include "snakeclient.h"
#include "common.h"

SnakeClient::SnakeClient(int& _clientID, std::vector<Powerup>& _powerups, std::unordered_map<uint32_t, Player>& _players, Grid* grid)
	: clientID(_clientID), powerups(_powerups), players(_players), gridPointer(grid)
{

}

void SnakeClient::UpdateServer()
{
	// Should update server with position information
}

void SnakeClient::HandleMessages()
{
	while (!Incoming().IsEmpty())
	{
		// Get msg
		net::Message<MessageTypes> msg = Incoming().PopFront().msg;

		std::cout << "HEADER: " << static_cast<std::underlying_type<MessageTypes>::type>(msg.header.id) << "\n";

		// Look at msg header to determine msg type
		switch (msg.header.id)
		{
		case MessageTypes::ServerValidation:
		{
			// Get client ID
			msg >> clientID;

			// Get powerups
			size_t powerupCount;
			msg >> powerupCount;
			powerups = std::vector<Powerup>(powerupCount);
			for (int c = 0; c < powerupCount; c++)
			{
				PowerupData powerup;
				msg >> powerup;
				powerups.emplace_back(powerup.x, powerup.y, (ImU32)ImColor(1.0f, 0.1f, 0.1f, 1.0f));
			}

			// Update players
			size_t playerCount;
			msg >> playerCount;
			std::cout << "Playercount after connection: " << playerCount << "\n";
			for (int c = 0; c < playerCount; c++)
			{
				// Get player ID
				int playerID;
				msg >> playerID;

				// Create player
				Player player = Player(gridPointer, ImColor(0.1f, 0.1f, 1.0f, 1.0f));

				// Get players tail
				size_t tailSize;
				msg >> tailSize;
				for (int i = 0; i < tailSize; i++)
				{
					Position pos;
					msg >> pos;
					player.tail.push_front(pos);
				}
				
				players[playerID] = player;
			}

			break;
		}

		case MessageTypes::NewPlayer:
		{
			// New player connected
			std::cout << "New player connected!\n";
			uint32_t newClientID;
			msg >> newClientID;

			Player newPlayer = Player(gridPointer, ImColor(0.1f, 0.1f, 1.0f, 1.0f));
			
			size_t tailSize;
			msg >> tailSize;

			for (int c = 0; c < tailSize; c++)
			{
				Position pos;
				msg >> pos;

				newPlayer.tail.push_front(pos);
			}

			players[newClientID] = newPlayer;
			break;
		}

		case MessageTypes::NewPowerup:
		{
			std::cout << "New powerup spawned\n";

			// Generate powerup locally
			PowerupData powerup;
			msg >> powerup;
			powerups.emplace_back(powerup.x, powerup.y, ImColor(1.0f, 0.1f, 0.1f, 1.0f));

			break;
		}
		}
	}
}

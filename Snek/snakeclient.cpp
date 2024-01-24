#include "pch.h"

#include "snakeclient.h"
#include "common.h"

SnakeClient::SnakeClient(int& _clientID, std::vector<Powerup>& _powerups, std::unordered_map<uint32_t, Player>& _players, Grid* grid)
	: clientID(_clientID), powerups(_powerups), players(_players), gridPointer(grid) { }

void SnakeClient::UpdateServer()
{
	// Updates server with position information
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::UpdatePlayer;

	// Send tail
	Player& player = players[clientID];
	
	for (Position& pos : player.tail)
	{
		msg << pos;
	}

	msg << player.tail.size();
	msg << player.GetDirection();

	// Push ID on top of stack
	msg << clientID;
	Send(msg);
}

void SnakeClient::HandleMessages()
{
	while (!Incoming().IsEmpty())
	{
		// Get msg
		net::Message<MessageTypes> msg = Incoming().PopFront().msg;

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

				ImU32 color;
				msg >> color;

				// Create player
				Player player = Player(gridPointer, color);

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

			ImU32 color;
			msg >> color;

			Player newPlayer = Player(gridPointer, color);
			
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

		case MessageTypes::RemovePlayer:
		{
			int id;
			msg >> id;

			std::cout << "Client disconnected: " << id << "\n";

			players.erase(id);

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

		case MessageTypes::UpdatePlayer:
		{
			std::cout << "UPDATE PLAYER\n";
			// Get player
			int newClientID;
			msg >> newClientID;
			Player& updatePlayer = players[newClientID];

			Direction dir;
			msg >> dir;
			updatePlayer.SetDirection(dir);

			updatePlayer.tail.clear();
			size_t tailSize;
			msg >> tailSize;

			for (int c = 0; c < tailSize; c++)
			{
				Position pos;
				msg >> pos;
				updatePlayer.tail.push_front(pos);
			}
			break;
		}

		case MessageTypes::PowerupEaten:
		{
			std::cout << "PowerupEaten\n";

			PowerupData powerup;
			msg >> powerup;

			// Delete powerup locally
			for (int c = 0; c < powerups.size(); c++)
			{
				if (powerups[c].x == powerup.x && powerups[c].y == powerup.y)
				{
					powerups.erase(powerups.begin() + c);
					break;
				}
			}
			break;
		}

		case MessageTypes::StartGame:
		{
			std::cout << "Game Started!\n";
			gameStarted = true;
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
}

void SnakeClient::EatPowerup(unsigned int c)
{
	// Save powerup before deleting
	PowerupData powerupEaten = PowerupData(powerups[c].x, powerups[c].y);

	// Use powerup
	players[clientID].AddLength(1);
	powerups.erase(powerups.begin() + c);

	// Update server of eaten powerup
	net::Message<MessageTypes> msg;
	msg.header.id = MessageTypes::PowerupEaten;
	msg << powerupEaten;
	Send(msg);
}

void SnakeClient::AttemptGameStart()
{
	if (!IsGameStarted())
	{
		std::cout << "I PRESSED P :O\n";
		net::Message<MessageTypes> msg;
		msg.header.id = MessageTypes::StartGame;

		// Updates gameStarted locally when server confirms that the game is starting
		Send(msg);
	}
}

bool SnakeClient::IsGameStarted() const
{
	return gameStarted;
}

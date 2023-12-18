#include <iostream>
#include "client.h"

#include "..\Networking\networking.h"
#include "..\SnekServer\common.h"

int main()
{
	// Create client instance and connect to server
	Client client;
	client.Connect(IP, PORT);
	
	// Main game loop
	bool run = true;
	while (run)
	{
		// Handle all messages from server
		client.HandleMessages();

		// GameGaming
	}

	// Exit everything
	client.Disconnect();

	return 0;
}

#include <unordered_map> // Til brug som liste af players?
//std::unordered_map<uint32_t, PLAYERDESCRIPTIONCLASS/STRUCT> players;

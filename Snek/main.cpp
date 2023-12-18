#include <iostream>
#include "client.h"

#include "..\Networking\networking.h"
#include "..\SnekServer\messageTypes.h"

#define IP "127.0.0.1"
#define PORT 8080

int main()
{
	// Create client instance and connect to server
	Client client;
	client.Connect(IP, PORT);
	
	// Run game here
	std::cout << "Hello World!\n";

	// Exit everything
	client.Disconnect();

	return 0;
}

#include <unordered_map> // Til brug som liste af players?
//std::unordered_map<uint32_t, PLAYERDESCRIPTIONCLASS/STRUCT> players;

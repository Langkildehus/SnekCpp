#include <iostream>
#include "server.h"

#include "..\Networking\networking.h"
#include "common.h"

int main()
{
	// Create server instance and start listening
	Server server(PORT);
	server.Start();

	// Main game loop
	bool run = true;
	while (run)
	{
		// Sleeps until msg is received from client
		server.Update(-1, true);
	}

	// Exit everything
	server.Stop();

	return 0;
}

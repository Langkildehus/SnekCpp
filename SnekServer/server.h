#pragma once

#include "..\Networking\networking.h"
#include "common.h"

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
};

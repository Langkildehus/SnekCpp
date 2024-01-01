#pragma once

#include "..\Networking\networking.h"
#include "common.h"

class SnakeClient : public net::ClientInterface<MessageTypes>
{
public:
	SnakeClient(int& _clientID);

	// Should be called to send update information to server
	void UpdateServer();

	// Should be called every frame to handle incoming messages
	void HandleMessages();

private:
	int& clientID;
};

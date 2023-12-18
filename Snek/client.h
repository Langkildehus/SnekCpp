#pragma once

#include "..\Networking\networking.h"
#include "..\SnekServer\common.h"

class Client : public net::ClientInterface<MessageTypes>
{
public:
	// Should be called to send update information to server
	void UpdateServer();

	// Should be called every frame to handle incoming messages
	void HandleMessages();
};

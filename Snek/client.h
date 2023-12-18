#pragma once

#include "..\Networking\networking.h"
#include "..\SnekServer\messageTypes.h"

class Client : public net::ClientInterface<MessageTypes>
{
public:
	void UpdateServer();
};

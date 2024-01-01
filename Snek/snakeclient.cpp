#include "snakeclient.h"
#include "common.h"

SnakeClient::SnakeClient(int& _clientID)
	: clientID(_clientID)
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
			msg >> clientID;
			std::cout << "ClientID received: " << clientID << "\n";
			break;
		}

		case MessageTypes::NewPlayer:
		{
			// New player connected
			break;
		}
		}
	}
}

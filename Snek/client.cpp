#include "client.h"

void Client::UpdateServer()
{
	// Should update server with position information
}

void Client::HandleMessages()
{
	while (!Incoming().IsEmpty())
	{
		// Get msg
		net::Message<MessageTypes> msg = Incoming().PopFront().msg;

		// Look at msg header to know how to handle msg
		switch (msg.header.id)
		{
		case MessageTypes::NewPlayer:
		{
			// New player connected
			break;
		}
		}
	}
}

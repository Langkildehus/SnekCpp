#include "server.h"

Server::Server(int port)
	: net::ServerInterface<MessageTypes>(port)
{

}

bool Server::OnClientConnect(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	// Allow all clients
	return true;
}

void Server::OnClientValidated(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	// Send some sort of ClientAccepted msg to the connected client
	// Probably also NewPlayer to all other clients
}

void Server::OnClientDisconnect(std::shared_ptr<net::Connection<MessageTypes>> client)
{
	// Send some sort of RemovePlayer msg to all clients
}

void Server::OnMessage(std::shared_ptr<net::Connection<MessageTypes>> client, net::Message<MessageTypes>& msg)
{

}

#pragma once

// IP and PORT to host and connect to
#define IP "172.0.0.1"
#define PORT 8080

// Types of messages to be sent/received over socket
enum class MessageTypes : uint32_t
{
	ServerAccept,     // Server accepts connection
	ServerValidation, // Server validated connection

	NewPlayer,        // New player connected
	RemovePlayer,     // Player left
	UpdatePlayer      // Player data
};

#pragma once

// Types of messages to be sent/received over socket
enum class MessageTypes : uint32_t
{
	ServerAccept,     // Server accepts connection
	ServerValidation, // Server validated connection

	NewPlayer,        // New player connected
	RemovePlayer,     // Player left
	UpdatePlayer      // Player data
};

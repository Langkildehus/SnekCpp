#pragma once

// IP and PORT to host and connect to
#define IP "127.0.0.1"
#define PORT 8080

// Grid
#define GRIDWIDTH 40
#define GRIDHEIGHT 30

// Types of messages to be sent/received over socket
enum class MessageTypes : uint32_t
{
	ServerValidation, // Server validated connection

	NewPlayer,        // New player connected
	RemovePlayer,     // Player left
	UpdatePlayer,     // Player data

	NewPowerup,       // Send to tell clients about a new powerup
	PowerupEaten      // Send when client eats powerup
};

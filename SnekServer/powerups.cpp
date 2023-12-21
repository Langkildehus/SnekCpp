#include "apple.h"

bool Powerup::CheckCollision(int _x, int _y)
{
	if (_x == x && _y == y)
		return true;

	return false;
}

// ----------- POWERUP ------------

Apple::Apple(int _x, int _y)
	: x(_x), y(_y)
{

}


// ------------ ORANGE --------------

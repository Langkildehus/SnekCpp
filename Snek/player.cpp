#include "player.h"
#include <vector>

Player::Player(int x, int y)
{
	Position pos{ x, y };
	tail.push_back(pos);
}

Position Player::GetHead()
{
	return tail[0];
}

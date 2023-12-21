#include "apple.h"

Apple::Apple(int _x, int _y)
	: x(_x), y(_y)
{

}

bool Apple::CheckCollision(int _x, int _y)
{
	if (_x == x && _y == y)
		return true;

	return false;
}

#pragma once

#include <vector>

struct Position
{
	int x, y;
};

class Player
{
public:
	Player(int x, int y);

	Position GetHead();

private:
	std::vector<Position> tail;
};

#pragma once

class Powerup
{
private:
	int x, y;
};

// Orange doubles snake speed
class Orange : Powerup
{
	Orange();
};

// +1 length
class Apple
{
public:
	Apple(int x, int y);

	bool CheckCollision(int x, int y);

private:
	int x, y;
};


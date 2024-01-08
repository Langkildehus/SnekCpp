#pragma once

#include <imgui.h>

#include "player.h"

struct PowerupData
{
	int x, y;
};

class Powerup
{
public:
	Powerup() = default;
	Powerup(int _x, int _y, ImU32 _color);

	bool CheckCollision(Position& pos);
	ImU32 GetColor();

public:
	int x, y;

private:
	ImU32 color;
};

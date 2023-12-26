#include <imgui.h>

#include "powerups.h"
#include "player.h"

Powerup::Powerup(int _x, int _y, ImU32 _color)
	: x(_x), y(_y), color(_color)
{

}

bool Powerup::CheckCollision(Position& pos)
{
	if (x == pos.x && y == pos.y)
		return true;

	return false;
}

ImU32 Powerup::GetColor()
{
	return color;
}

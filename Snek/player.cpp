#include "pch.h"

#include "player.h"
#include "grid.h"

Player::Player(Grid* _grid, ImU32 _color)
	: grid(_grid), direction(Direction::Up), color(_color) { }

void Player::Move()
{
	switch (direction)
	{
	case Direction::Up:
	{
		tail.emplace_front(
			tail[0].x,
			((tail[0].y - 1) + (*grid).GetRowCount()) % (*grid).GetRowCount()
		);

		UpdateTail();
	}
	break;

	case Direction::Right:
	{
		tail.emplace_front(
			((tail[0].x + 1) + (*grid).GetColCount()) % (*grid).GetColCount(),
			tail[0].y
		);

		UpdateTail();
	}
	break;

	case Direction::Down:
	{
		tail.emplace_front(
			tail[0].x,
			((tail[0].y + 1) + (*grid).GetRowCount()) % (*grid).GetRowCount()
		);

		UpdateTail();
	}
	break;

	case Direction::Left:
	{
		tail.emplace_front(
			((tail[0].x - 1) + (*grid).GetColCount()) % (*grid).GetColCount(),
			tail[0].y
		);

		UpdateTail();
	}
	break;
	}
}

void Player::AddLength(int l)
{
	food += l;
}

void Player::SetDirection(Direction newDirection)
{
	direction = newDirection;
}

Position& Player::GetHead()
{
	return tail[0];
}

ImU32 Player::GetColor() const
{
	return color;
}

Direction Player::GetDirection() const
{
	return direction;
}

void Player::UpdateTail()
{
	// Remove last tail piece if no food has been eaten
	if (food > 0)
	{
		food--;
	}
	else
	{
		tail.pop_back();
	}
}

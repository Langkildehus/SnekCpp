#include "pch.h"

#include "player.h"
#include "grid.h"

Player::Player(Grid* _grid, ImU32 _color)
	: grid(_grid), direction(Direction::Up), color(_color) { }

void Player::Move()
{
	// Don't move if dead
	if (!alive)
	{
		return;
	}

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

void Player::Kill()
{
	alive = false;
	tail = std::deque<Position>();
}

bool Player::IsAlive() const
{
	return alive;
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

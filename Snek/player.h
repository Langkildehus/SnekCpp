#pragma once

#include <deque>

#include <imgui.h>

// Forward declare Grid
class Grid;

enum class Direction
{
	Up,
	Right,
	Down,
	Left
};

struct Position
{
	int x, y;
};

struct PlayerData
{
	Direction direction;
	std::deque<Position> tail;
	ImU32 color;
};

class Player
{
public:
	Player() = default;
	Player(Grid* _grid, ImU32 _color);

	void Move();
	void AddLength(int l);
	void SetDirection(Direction newDirection);

	Position& GetHead();
	ImU32 GetColor();
	Direction GetDirection();

private:
	void UpdateTail();

public:
	std::deque<Position> tail;

private:
	int food = 0;

	Direction direction;

	Grid* grid;

	ImU32 color;
};

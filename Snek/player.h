#pragma once

#include <deque>

#include "imgui\imgui.h"

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

class Player
{
public:
	Player() = default;
	Player(int x, int y, Grid* _grid);

	void Move();
	void AddLength(int l);
	void SetDirection(Direction newDirection);

	Position& GetHead();
	ImU32 GetColor();

private:
	void UpdateTail();

public:
	std::deque<Position> tail;

private:
	int food = 0;

	Direction direction = Direction::Up;

	Grid* grid;

	ImU32 color;
};

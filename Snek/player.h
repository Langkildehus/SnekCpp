#pragma once

// Forward declare Grid
class Grid;

enum class Direction : uint32_t
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
	void Kill();

	bool IsAlive() const;
	Position& GetHead();
	ImU32 GetColor() const;
	Direction GetDirection() const;

private:
	void UpdateTail();

public:
	std::deque<Position> tail;

private:
	bool alive = true;

	int food = 0;

	Direction direction;

	Grid* grid;

	ImU32 color;
};

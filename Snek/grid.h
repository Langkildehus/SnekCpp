#pragma once

#include "player.h"
#include "powerups.h"

class Grid
{
public:
	Grid(int _rows, int _cols, int _size, bool* _mainLoop, int sWidth, int sHeight, std::unordered_map<uint32_t, Player>& _players, std::vector<Powerup>& _powerups);

	void Render();

	int GetRowCount() const;
	int GetColCount() const;

private:
	void DrawGrid();
	void DrawPowerups();
	void DrawSnakes();
	void DrawBorder();
	void RenderFrame();

private:
	int rows, cols, size;
	int startX, startY, width, height;
	bool* mainLoop;

	std::unordered_map<uint32_t, Player>& players;

	std::vector<Powerup>& powerups;

	const ImU32 WHITE = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
};

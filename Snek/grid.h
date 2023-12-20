#pragma once

class Grid
{
public:
	Grid(int _rows, int _cols, int _size);

	void Render();

private:
	void Draw();
	void RenderFrame();

private:
	int rows, cols, size;
	int startX, startY, width, height;
};

#pragma once

class Grid
{
public:
	Grid(int _rows, int _cols, int _size);

	void Render(int mark);

private:
	void Draw(int mark);
	void RenderFrame();

private:
	int rows, cols, size;
	int startX, startY, width, height;
};

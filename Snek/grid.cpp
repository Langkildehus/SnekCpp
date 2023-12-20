#include "..\..\imgui\imgui.h"

#include "grid.h"
#include "gui.h"

Grid::Grid(int _rows, int _cols, int _size)
	: rows(_rows), cols(_cols), size(_size)
{
	startX = gui::WIDTH / 2 - size * rows / 2;
	startY = gui::HEIGHT / 2 - size * cols / 2;

	width = rows * size + 100;
	height = cols * size + 100;
}

void Grid::Draw()
{
	const ImU32 color = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	for (int x = 0; x < rows; x++)
	{
		for (int y = 0; y < cols; y++)
		{
			//ImVec2 topLeft = ImVec2(x * size, y * size);
			//ImVec2 botRight = ImVec2((x + 1) * size, (y + 1) * size);

			ImVec2 topLeft = ImVec2(x * size + startX + 1, y * size + startY + 1);
			ImVec2 botRight = ImVec2((x + 1) * size + startX + 1, (y + 1) * size + startY + 1);
			ImGui::GetWindowDrawList()->AddRect(topLeft, botRight, color);
		}
	}
}

void Grid::Render()
{
	RenderFrame();
	Draw();
	ImGui::End();
}

void Grid::RenderFrame()
{
	ImGui::SetNextWindowPos({ (float)(startX - 50), (float)(startY - 50) });
	ImGui::SetNextWindowSize({ (float)(width + 100), (float)(height + 100) });
	ImGui::Begin(
		"SnakeBattleRoyale",
		&gui::exit,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);
}

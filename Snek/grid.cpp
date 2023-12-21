#include "..\..\imgui\imgui.h"

#include "grid.h"
#include "gui.h"

Grid::Grid(int _rows, int _cols, int _size)
	: rows(_rows), cols(_cols), size(_size)
{
	startX = gui::WIDTH / 2 - size * (rows + 1) / 2 + 50;
	startY = gui::HEIGHT / 2 - size * (cols + 1) / 2 + 50;

	width = rows * size + 100;
	height = cols * size + 100;
}

void Grid::Draw(int mark)
{
	const ImU32 color = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	for (int x = 0; x < rows; x++)
	{
		for (int y = 0; y < cols; y++)
		{
			ImVec2 topLeft = ImVec2(x * size + startX, y * size + startY);
			ImVec2 botRight = ImVec2((x + 1) * size + startX, (y + 1) * size + startY);

			if (y == mark)
				ImGui::GetWindowDrawList()->AddRectFilled(topLeft, botRight, color);
			else
				ImGui::GetWindowDrawList()->AddRect(topLeft, botRight, color);
		}
	}
}

void Grid::Render(int mark)
{
	RenderFrame();
	Draw(mark);
	ImGui::End();
}

void Grid::RenderFrame()
{
	ImGui::SetNextWindowPos({ (float)startX - 50, 50.0f });
	ImGui::SetNextWindowSize({ (float)width, (float)height });
	ImGui::Begin(
		"SnakeBattleRoyale",
		&gui::exit,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);
}

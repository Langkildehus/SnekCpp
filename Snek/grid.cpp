#include <vector>
#include <unordered_map>

#include "imgui\imgui.h"

#include "grid.h"
#include "gui.h"
#include "powerups.h"

Grid::Grid(int _cols, int _rows, int _size, std::unordered_map<uint32_t, Player>& _players, std::vector<Powerup>& _powerups)
	: cols(_cols), rows(_rows), size(_size), players(_players), powerups(_powerups)
{
	startX = gui::WIDTH / 2 - size * (cols + 0) / 2;
	startY = gui::HEIGHT / 2 - size * (rows + 0) / 2 + 15;

	width = cols * size + 100;
	height = rows * size + 130;
}

int Grid::GetRowCount()
{
	return rows;
}

int Grid::GetColCount()
{
	return cols;
}

void Grid::DrawGrid()
{
	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	for (int x = 0; x < cols; x++)
	{
		for (int y = 0; y < rows; y++)
		{
			ImVec2 topLeft = ImVec2((float)(x * size + startX), (float)(y * size + startY));
			ImVec2 botRight = ImVec2((float)((x + 1) * size + startX), (float)((y + 1) * size + startY));
			ImGui::GetWindowDrawList()->AddRect(topLeft, botRight, WHITE);
		}
	}
}

void Grid::DrawPowerups()
{
	for (Powerup& powerup : powerups)
	{
		ImVec2 topLeft = ImVec2((float)(powerup.x * size + startX), (float)(powerup.y * size + startY));
		ImVec2 botRight = ImVec2((float)((powerup.x + 1) * size + startX), (float)((powerup.y + 1) * size + startY));
		ImGui::GetWindowDrawList()->AddRectFilled(topLeft, botRight, powerup.GetColor());
	}
}

void Grid::DrawSnakes()
{
	for (std::pair<const uint32_t, Player>& player : players)
	{
		for (Position& pos : player.second.tail)
		{
			ImVec2 topLeft = ImVec2((float)(pos.x * size + startX), (float)(pos.y * size + startY));
			ImVec2 botRight = ImVec2((float)((pos.x + 1) * size + startX), (float)((pos.y + 1) * size + startY));

			ImGui::GetWindowDrawList()->AddRectFilled(topLeft, botRight, player.second.GetColor());
		}
	}
}

void Grid::DrawBorder()
{
	ImVec2 topLeft = ImVec2((float)startX, (float)startY);
	ImVec2 botRight = ImVec2((float)(startX + cols * size), (float)(startY + rows * size));
	ImGui::GetWindowDrawList()->AddRect(topLeft, botRight, WHITE);
}

void Grid::Render()
{
	RenderFrame();

	//DrawGrid();
	DrawPowerups();
	DrawSnakes();
	DrawBorder();

	ImGui::End();
}

void Grid::RenderFrame()
{
	//ImGui::SetNextWindowPos({ (float)startX - 50, 50.0f });
	ImGui::SetNextWindowPos({ 0.0f, 0.0f });
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

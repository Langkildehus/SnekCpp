#pragma once

class Gui
{
public:
	GLFWwindow* Init(int _width, int _height, const char* title);
	void NewFrame();
	void Render();
	void Shutdown();
};

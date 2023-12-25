#pragma once

#include "glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Gui
{
public:
	GLFWwindow* Init(int _width, int _height, const char* title);
	void NewFrame();
	void Update();
	void Render();
	void Shutdown();
};

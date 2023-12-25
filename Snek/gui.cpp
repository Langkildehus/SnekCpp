#include "glad\glad.h"
#include "glfw3.h"

#include "gui.h"

GLFWwindow* Gui::Init(int _width, int _height, const char* title)
{
	if (!glfwInit())
		return nullptr;

	// GL 3.0 & GLSL 130
	const char* glslVersion = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics
	GLFWwindow* window = glfwCreateWindow(_width, _height, title, NULL, NULL);
	if (window == NULL)
		return nullptr;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // ENABLE VSYNC

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // Tie window context to glad's opener
		throw("Unable to tie GLAD to OpenGL");

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Setup Platform/Render bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);
	ImGui::StyleColorsDark();

	return window;
}

void Gui::NewFrame()
{
	// Feed inputs to ImGui - Start new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Gui::Update()
{
	// Create ImGui DrawData
	ImGui::Begin("BEGIN FROM IMGUI");

	ImGui::Text("Application average %.3f msg/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

void Gui::Render()
{
	// Render ImGui to screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::Shutdown()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

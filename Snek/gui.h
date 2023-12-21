#pragma once

#include <d3d9.h>

namespace gui
{
	// Constant window size
	constexpr inline int WIDTH = 1300;
	constexpr inline int HEIGHT = 1030;

	inline bool exit = true;

	// Winapi for window
	inline HWND window = nullptr;
	inline WNDCLASSEXA windowClass = { };

	// Points for window movement
	inline POINTS position = { };

	// Directx state variables
	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS presentParameters = { };

	// Handle window creation and destruction
	void CreateHWindow(const char* windowName, const char* className) noexcept;
	void DestroyHWindow() noexcept;

	// Handle device creation and destruction
	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	// Handle ImGui creation and destruction
	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;

	// My additions
	void Init(const char* windowName, const char* className) noexcept;
	void Destroy() noexcept;
}
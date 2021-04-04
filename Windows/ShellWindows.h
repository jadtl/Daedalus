#pragma once

#include "Shell.h"

class ShellWindows : Shell {
public:
	void createWindow();
	VkSurfaceKHR createSurface(VkInstance instance);

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		ShellWindows* shell = reinterpret_cast<ShellWindows*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!shell) return DefWindowProc(hwnd, uMsg, wParam, lParam);

		return shell -> handleMessage(uMsg, wParam, lParam);
	}
	LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);

private:
	HINSTANCE hInstance;
	HWND hWnd;
	HMODULE hModule;
};
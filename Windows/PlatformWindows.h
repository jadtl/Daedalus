#pragma once

#include <Windows.h>

#include "Platform.h"

class PlatformWindows : public Platform {
public:
	void createWindow();
	VkSurfaceKHR createSurface(VkInstance instance);

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		PlatformWindows* platform = reinterpret_cast<PlatformWindows*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (!platform) return DefWindowProc(hwnd, uMsg, wParam, lParam);

		return platform -> handleMessage(uMsg, wParam, lParam);
	}
	LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);

private:
	HINSTANCE hInstance;
	HWND hWnd;
	HMODULE hModule;
};

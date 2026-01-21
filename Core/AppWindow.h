#pragma once

struct AppWindow {
	int Width;
	int Height;

	AppWindow(int width, int height, const char* title = "Vulkan Framework");
	~AppWindow();

	struct GLFWwindow* const GetWindow() const;

private:
	struct GLFWwindow* hWnd;
};
#pragma once

struct AppWindow {
	int Width;
	int Height;

	void setup(int width, int height, const char* title = "Vulkan Framework");
	void teardown();

	struct GLFWwindow* const GetWindow() const;

private:
	struct GLFWwindow* hWnd;
};
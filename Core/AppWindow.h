#pragma once

typedef unsigned int uint32_t;
struct GLFWwindow;

typedef void(*const FrameBufferResizeCallbackDelegate)(GLFWwindow*, int, int);

struct AppWindow {
	int Width;
	int Height;

	AppWindow* const getInstance();

	void setup(int width, int height, FrameBufferResizeCallbackDelegate& resizeCallback, const char* title = "Vulkan Framework");
	void teardown();

	struct GLFWwindow* const GetWindow() const;
private:
	static AppWindow* instance;
	struct GLFWwindow* hWnd;
};
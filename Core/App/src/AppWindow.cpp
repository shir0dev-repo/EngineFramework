#include "../AppWindow.h"
#include <GLFW/glfw3.h>

AppWindow* AppWindow::instance = nullptr;

AppWindow* const AppWindow::getInstance() {
	if (instance == nullptr) {
		instance = new AppWindow();
	}

	return instance;
}

void AppWindow::setup(int width, int height, FrameBufferResizeCallbackDelegate& resizeCallback, const char* title) {
	this->Width = width;
	this->Height = height;
	this->hWnd = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwSetFramebufferSizeCallback(hWnd, resizeCallback);
}

void AppWindow::teardown() {
	if (this->hWnd != nullptr) {
		glfwDestroyWindow(this->hWnd);
		this->hWnd = nullptr;
	}
	delete instance;
}

GLFWwindow* const AppWindow::GetWindow() const {
	return hWnd;
}
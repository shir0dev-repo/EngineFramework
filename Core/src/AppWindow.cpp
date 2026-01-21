#include "../AppWindow.h"
#include <GLFW/glfw3.h>

AppWindow::AppWindow(int width, int height, const char* title) {
	this->Width = width;
	this->Height = height;
	this->hWnd = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

AppWindow::~AppWindow() {
	if (this->hWnd != nullptr) {
		glfwDestroyWindow(this->hWnd);
		this->hWnd = nullptr;
	}
}

GLFWwindow* const AppWindow::GetWindow() const {
	return hWnd;
}
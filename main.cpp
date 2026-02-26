#define GLFW_INCLUDE_VULKAN

#include "Core/App/Application.h"

#include <GLFW/glfw3.h>
#include <iostream>
int main() {
	Application* app = Application::getInstance();
	return app->run();
}
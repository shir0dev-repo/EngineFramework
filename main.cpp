#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "Core/Application.h"
int main() {
	Application* app = Application::getInstance();
	return app->run();
}
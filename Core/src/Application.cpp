#include "../Application.h"
#include "../AppWindow.h"
#include "../VulkanInstance.h"

#include <GLFW/glfw3.h>

int Application::run() {
	if (!initWindow()) {
		return -1;
	}

	initVulkan();
	mainLoop();
	cleanup();

	return 0;
}

int Application::initWindow() {
	if (glfwInit() == GLFW_FALSE) {
		glfwTerminate();
		return 0;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't make an openGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Set resizing to false, for now
	
	this->window = new AppWindow(800, 800);

	return 1;
}

void Application::initVulkan() {
	this->vkInstance = new VulkanInstance();
}

void Application::mainLoop() {
	while (!glfwWindowShouldClose(window->GetWindow())) {
		glfwPollEvents();
	}
}

void Application::cleanup() {
	delete vkInstance;
	delete window;
	glfwTerminate();
}
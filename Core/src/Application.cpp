#include "../Application.h"
#include "../AppWindow.h"
#include "../Vulkan/VulkanInstance.h"
#include "../Vulkan/VulkanValidator.h"
#include "../Vulkan/VulkanDevice.h"
#include "../Vulkan/VulkanSwapChain.h"
#include "../Graphics/GraphicsPipeline.h"

#include <GLFW/glfw3.h>

int Application::run() {
	if (!initWindow()) {
		return -1;
	}

	initVulkan();
	initGraphicsPipeline();
	mainLoop();
	cleanup();

	return 0;
}

bool Application::initWindow() {
	if (glfwInit() == GLFW_FALSE) {
		glfwTerminate();
		return 0;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't make an openGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Set resizing to false, for now
	
	this->window = new AppWindow();
	window->setup(800, 800);

	return 1;
}

void Application::initVulkan() {
	this->vkInstance = new VulkanInstance();
	this->vkInstance->setup(this->window->GetWindow());
}

void Application::initGraphicsPipeline() {
	this->graphicsPipeline = new GraphicsPipeline();
	this->graphicsPipeline->setup(vkInstance->device->logicalDevice, vkInstance->swapChain);
}

void Application::mainLoop() {
	while (!glfwWindowShouldClose(window->GetWindow())) {
		glfwPollEvents();
	}
}

void Application::cleanup() {
	graphicsPipeline->teardown(vkInstance->device->logicalDevice);
	vkInstance->teardown();
	delete vkInstance;
	window->teardown();
	delete window;
	
	glfwTerminate();
}
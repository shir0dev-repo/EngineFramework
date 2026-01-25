#include "../Application.h"
#include "../AppWindow.h"
#include "../Vulkan/VulkanInstance.h"
#include "../Vulkan/VulkanValidator.h"
#include "../Vulkan/VulkanDevice.h"
#include "../Vulkan/VulkanSwapChain.h"
#include "../Graphics/GraphicsPipeline.h"
#include "../Graphics/Shader/Vertex.h"
#include "../Graphics/Renderer/MeshRenderer.h"
#include "../Graphics/Mesh/Mesh.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

static std::vector<Vertex> vertices = {
	{{  0.0f,   0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }},
	{{ -0.5f,  -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }},
	{{  0.5f,  -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }}
};

Application* Application::instance = nullptr;

int Application::run() {
	if (instance == nullptr) {
		instance = this;
	}
	else {
		throw;
	}

	if (!initWindow()) {
		return -1;
	}

	initVulkan();
	initGraphicsPipeline();
	mainLoop();
	cleanup();

	return 0;
}

void Application::onWindowResized(GLFWwindow* window, int width, int height) {
	if (instance->graphicsPipeline == nullptr) {
		return;
	}

	GraphicsPipeline::onWindowResized(instance->graphicsPipeline, window, width, height);
}

bool Application::initWindow() {
	if (glfwInit() == GLFW_FALSE) {
		glfwTerminate();
		return 0;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't make an openGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	this->window = new AppWindow();
	window->setup(800, 800, &Application::onWindowResized);

	return 1;
}

void Application::initVulkan() {
	this->vkInstance = new VulkanInstance();
	this->vkInstance->setup(this->window->GetWindow());
}

void Application::initGraphicsPipeline() {
	this->graphicsPipeline = new GraphicsPipeline();
	this->graphicsPipeline->setup(vkInstance->device, vkInstance->swapChain, vkInstance->surface);
}

void Application::mainLoop() {
	Mesh* mesh = new Mesh();
	mesh->vertexData = vertices.data();
	mesh->vertexCount = vertices.size();
	MeshRenderer* renderer = new MeshRenderer();
	renderer->setup(vkInstance->device, mesh);

	while (!glfwWindowShouldClose(window->GetWindow())) {
		glfwPollEvents();
		renderer->draw(graphicsPipeline);
		graphicsPipeline->render(vkInstance->device, vkInstance->surface, window->GetWindow());
	}

	vkDeviceWaitIdle(vkInstance->device->logicalDevice);

	renderer->teardown(vkInstance->device->logicalDevice);
	delete renderer;
	delete mesh;
}

void Application::cleanup() {
	graphicsPipeline->teardown(vkInstance->device->logicalDevice);
	delete graphicsPipeline;
	vkInstance->teardown();
	delete vkInstance;
	window->teardown();
	delete window;
	
	glfwTerminate();
}
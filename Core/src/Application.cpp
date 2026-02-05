#include "../Application.h"
#include "../AppWindow.h"
#include "../Vulkan/VulkanInstance.h"
#include "../Vulkan/VulkanValidator.h"
#include "../Vulkan/VulkanDevice.h"
#include "../Vulkan/VulkanSwapChain.h"
#include "../Graphics/Pipeline/GraphicsPipeline.h"
#include "../Graphics/Pipeline/RenderPass.h"
#include "../Graphics/Shader/Vertex.h"
#include "../Graphics/Renderer/MeshRenderer.h"
#include "../Graphics/Mesh/Mesh.h"
#include "../Graphics/Shader/ShaderModule.h"
#include "../Graphics/Shader/PipelineShader.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Component/Camera.h"
#include "../Graphics/Texture/GPUTexture.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

static std::vector<Vertex> vertices = {
	{{ -0.5f,  -0.5f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }},
	{{  0.5f,  -0.5f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }},
	{{  0.5f,   0.5f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }},
	{{ -0.5f,   0.5f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.0f }}
};

static std::vector<uint32_t> indices = {
	0, 1, 2,
	2, 3, 0
};
static GPUTexture* defaultTexture; 

Application* const Application::getInstance() {
	static Application* instance = nullptr;
	if (instance == nullptr) {
		instance = new Application();
	}

	return instance;
}

int Application::run() {
	if (!initWindow()) {
		return -1;
	}

	initAssets();
	initVulkan();
	initRenderer();
	mainLoop();
	cleanup();

	return 0;
}

void Application::onWindowResized(GLFWwindow* window, int width, int height) {
	getInstance()->renderer->notifyFramebufferResized();
}

bool Application::initWindow() {
	if (glfwInit() == GLFW_FALSE) {
		glfwTerminate();
		return 0;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't make an openGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	this->window = AppWindow::getInstance();
	window->setup(800, 800, &Application::onWindowResized);

	return 1;
}

void Application::initAssets() {
	defaultTexture = GPUTexture::createTexture("Assets/Textures/texture.jpg", "default");
}

void Application::initVulkan() {
	this->vkInstance = VulkanInstance::getInstance();
	this->vkInstance->setup(this->window->GetWindow());
}

void Application::initRenderer() {
	this->renderer = Renderer::getInstance();
	renderer->setup(vkInstance);

	ShaderModule* vertex = ShaderModule::createNew(vkInstance->device->logicalDevice, "Assets/Shaders/vert.spv", "default-v");
	ShaderModule* fragment = ShaderModule::createNew(vkInstance->device->logicalDevice, "Assets/Shaders/frag.spv", "default-f");
	PipelineShader shader = {};
	shader.vertexModule = vertex;
	shader.fragmentModule = fragment;

	renderer->addPipeline(&shader);

	GPUTexture::loadGPU(vkInstance, defaultTexture);
}

void Application::mainLoop() {
	Mesh* mesh = new Mesh();
	mesh->vertexData = vertices.data();
	mesh->vertexCount = vertices.size();
	mesh->indexData = indices.data();
	mesh->indexCount = indices.size();

	MeshRenderer* meshRenderer = new MeshRenderer();
	meshRenderer->setup(vkInstance->device, mesh, renderer);

	while (!glfwWindowShouldClose(window->GetWindow())) {
		glfwPollEvents();
		
		meshRenderer->draw(renderer->getPipeline(nullptr));
		renderer->render(vkInstance, window->GetWindow());
	}
	
	vkDeviceWaitIdle(vkInstance->device->logicalDevice);

	meshRenderer->teardown(vkInstance->device->logicalDevice);
	delete meshRenderer;
	delete mesh;
}

void Application::cleanup() {
	GPUTexture::cleanup(vkInstance);
	ShaderModule::teardown(vkInstance->device->logicalDevice);
	renderer->teardown(vkInstance->device->logicalDevice);
	delete renderer;
	vkInstance->teardown();
	delete vkInstance;
	window->teardown();
	
	glfwTerminate();
}
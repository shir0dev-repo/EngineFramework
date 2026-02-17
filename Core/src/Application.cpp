#include "Core/Structure/IDVector.h"
#include "../Application.h"
#include "../AppWindow.h"
#include "../Vulkan/VulkanInstance.h"
#include "../Vulkan/VulkanValidator.h"
#include "../Vulkan/VulkanDevice.h"
#include "../Vulkan/VulkanSwapChain.h"
#include "../Graphics/Pipeline/GraphicsPipeline.h"
#include "../Graphics/Shader/Vertex.h"
#include "../Graphics/Renderer/MeshRenderer.h"
#include "../Graphics/Mesh/Mesh.h"
#include "../Graphics/Material/Material.h"
#include "../Graphics/Shader/ShaderModule.h"
#include "../Graphics/Shader/PipelineShader.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Component/Camera.h"
#include "../Graphics/Texture/GPUTexture.h"
#include "Core/Graphics/Mesh/Util/MeshLoader.h"
#include "Runtime/Scene/Entity.h"
#include "Runtime/Scene/SceneNode.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

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
	static Application* instance = getInstance();
	
	instance->renderer->notifyFramebufferResized();
	instance->window->Width = width;
	instance->window->Height = height;
}

bool Application::initWindow() {
	if (glfwInit() == GLFW_FALSE) {
		glfwTerminate();
		return 0;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	this->window = AppWindow::getInstance();
	window->setup(800, 800, &Application::onWindowResized);

	return 1;
}

void Application::initAssets() {
	GPUTexture::createTexture("Assets/Textures/texture.jpg", "default");
	GPUTexture::createTexture("Assets/Textures/uv-checker.png", "shipTexture");
}

void Application::initVulkan() {
	this->vkInstance = VulkanInstance::getInstance();
	this->vkInstance->setup(this->window->GetWindow());
}

static void uploadTextures(VulkanInstance* vkInstance);

void Application::initRenderer() {
	this->renderer = Renderer::getInstance();
	renderer->setup(vkInstance);
	int vertexID = ShaderModule::createNew(vkInstance->device->logicalDevice, "Assets/Shaders/vert.spv", "default-v");
	int fragmentID = ShaderModule::createNew(vkInstance->device->logicalDevice, "Assets/Shaders/frag.spv", "default-f");

	ShaderModule* vertex = nullptr; 
	ShaderModule::find(vertexID, &vertex);
	ShaderModule* fragment = nullptr;
	ShaderModule::find(fragmentID, &fragment);
	
	PipelineShader shader = {};
	shader.vertexModule = vertex;
	shader.fragmentModule = fragment;

	renderer->addPipeline(&shader);

	uploadTextures(vkInstance);
}

void Application::mainLoop() {
	Mesh* mesh = nullptr;
	MeshLoader::loadOBJ("Assets/OBJ/ship.obj", &mesh);
	
	Material* material = Material::create(vkInstance, renderer->getPipeline(nullptr), "default");

	GPUTexture* shipTexture = nullptr;
	GPUTexture::getTexture("shipTexture", &shipTexture);
	material->setTexture(vkInstance, shipTexture, 0);
	
	MeshRenderer* meshRenderer = new MeshRenderer();
	Entity* entity = new Entity();
	entity->setPosition({ 0, 0, 5 });
	meshRenderer->setup(vkInstance, entity, mesh, material, renderer);
	float time = 0;

	while (!glfwWindowShouldClose(window->GetWindow())) {
		time += 0.01f;
		glfwPollEvents();
		shml::vec3f inputDir{};
		if (glfwGetKey(window->GetWindow(), GLFW_KEY_A) == GLFW_PRESS) {
			inputDir.x = -1;
		}
		else if (glfwGetKey(window->GetWindow(), GLFW_KEY_D) == GLFW_PRESS) {
			inputDir.x = 1;
		}
		if (glfwGetKey(window->GetWindow(), GLFW_KEY_S) == GLFW_PRESS) {
			inputDir.z = -1;
		}
		else if (glfwGetKey(window->GetWindow(), GLFW_KEY_W) == GLFW_PRESS) {
			inputDir.z = 1;
		}

		inputDir = inputDir.normalized_safe() * 0.02f;
		meshRenderer->entity->setPosition(entity->getPosition() + inputDir);
		const float* transform = entity->getTransform().getPointer();
		material->setBuffer(vkInstance, 3, 0, transform, sizeof(shml::matrix4f));
		meshRenderer->draw(renderer->getPipeline(nullptr));
		renderer->render(vkInstance, window->GetWindow());
	}
	
	vkDeviceWaitIdle(vkInstance->device->logicalDevice);

	meshRenderer->teardown(vkInstance->device->logicalDevice);
	delete meshRenderer;
	delete entity;
	delete mesh;
}

void Application::cleanup() {
	Material::cleanup(vkInstance);
	GPUTexture::cleanup(vkInstance);
	ShaderModule::teardown(vkInstance->device->logicalDevice);
	renderer->teardown(vkInstance->device->logicalDevice);
	delete renderer;
	vkInstance->teardown();
	delete vkInstance;
	window->teardown();
	
	glfwTerminate();
}

void uploadTextures(VulkanInstance* vkInstance) {
	GPUTexture* texture;
	GPUTexture::getTexture("default", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("shipTexture", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
}
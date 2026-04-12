#include "Core/Structure/IDVector.h"
#include "../Application.h"
#include "../AppWindow.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"
#include "Core/Graphics/Pipeline/GraphicsPipeline.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Entity/Component/MeshRenderer.h"
#include "Core/Scene/RenderNode.h"
#include "Core/Scene/NodeMover.h"

#include "Core/Graphics/Material/Material.h"
#include "Core/Graphics/Shader/ShaderModule.h"
#include "Core/Graphics/Shader/PipelineShader.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Entity/Component/Camera.h"
#include "Core/Graphics/Texture/GPUTexture.h"
#include "Core/Graphics/Mesh/Util/MeshLoader.h"
#include "Core/Entity/Entity.h"
#include "Core/Scene/SceneNode.h"
#include "Core/Entity/CommandBuffer/EntityCommandBuffer.h"
#include "Core/Scene/World.h"
#include "Core/Input/InputHandler.h"
#include "Core/Events/EventHandler.h"
#include "Core/UI/FontAsset.h"
#include "Core/UI/TextMesh.h"

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
	initMaterials();
	mainLoop();
	cleanup();

	return 0;
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

	glfwSetKeyCallback(window->GetWindow(), &InputHandler::GLFWkeyCallback);

	return 1;
}

void Application::initAssets() {
	GPUTexture::createTexture("Assets/Textures/texture.jpg", "default");
	GPUTexture::createTexture("Assets/Textures/uv-checker.png", "shipTexture");

	GPUTexture::createTexture("Assets/Textures/Skybox/skybox-top.png", "skybox-top");
	GPUTexture::createTexture("Assets/Textures/Skybox/skybox-front.png", "skybox-front");
	GPUTexture::createTexture("Assets/Textures/Skybox/skybox-right.png", "skybox-right");
	GPUTexture::createTexture("Assets/Textures/Skybox/skybox-back.png", "skybox-back");
	GPUTexture::createTexture("Assets/Textures/Skybox/skybox-left.png", "skybox-left");
	GPUTexture::createTexture("Assets/Textures/Skybox/skybox-bottom.png", "skybox-bottom");
}

void Application::initVulkan() {
	this->vkInstance = VulkanContext::getInstance();
	this->vkInstance->setup(this->window->GetWindow());
}

static void uploadTextures(VulkanContext* vkInstance);

void Application::initRenderer() {
	this->renderer = Renderer::getInstance();
	renderer->setup(vkInstance);
	int vertexID = ShaderModule::createNew(vkInstance->getDevice()->getLogicalDevice(), "Assets/Shaders/vert.spv", "default-v");
	int fragmentID = ShaderModule::createNew(vkInstance->getDevice()->getLogicalDevice(), "Assets/Shaders/frag.spv", "default-f");

	ShaderModule* vertex = nullptr; 
	ShaderModule::find(vertexID, &vertex);
	ShaderModule* fragment = nullptr;
	ShaderModule::find(fragmentID, &fragment);
	
	PipelineShader shader = {};
	shader.vertexModule = vertex;
	shader.fragmentModule = fragment;

	renderer->addPipeline(&shader, false);

	int skyboxVertexID = ShaderModule::createNew(vkInstance->getDevice()->getLogicalDevice(), "Assets/Shaders/skybox-vert.spv", "skybox-v");
	int skyboxFragmentID = ShaderModule::createNew(vkInstance->getDevice()->getLogicalDevice(), "Assets/Shaders/skybox-frag.spv", "skybox-f");

	ShaderModule::find(skyboxVertexID, &vertex);
	ShaderModule::find(skyboxFragmentID, &fragment);

	PipelineShader skyboxShader = {};
	skyboxShader.vertexModule = vertex;
	skyboxShader.fragmentModule = fragment;
	renderer->addPipeline(&skyboxShader, false);

	int fontVertexID = ShaderModule::createNew(vkInstance->getDevice()->getLogicalDevice(), "Assets/Shaders/UIText-vert.spv", "font-v");
	int fontFragmentID = ShaderModule::createNew(vkInstance->getDevice()->getLogicalDevice(), "Assets/Shaders/UIText-frag.spv", "font-f");

	ShaderModule::find(fontVertexID, &vertex);
	ShaderModule::find(fontFragmentID, &fragment);

	PipelineShader fontShader = {};
	fontShader.vertexModule = vertex;
	fontShader.fragmentModule = fragment;
	renderer->addPipeline(&fontShader, true);

	uploadTextures(vkInstance);
}

void Application::initMaterials() {
	Material* material = Material::create(vkInstance, renderer->getPipeline(0), "default");
	Material* skyboxMaterial = Material::create(vkInstance, renderer->getPipeline(1), "skybox");
	
	FontAsset* font = FontAsset::create(vkInstance, "Assets/Fonts/Minecraft.ttf", 16.0f);
	Material* fontMaterial = Material::create(vkInstance, renderer->getTransparentPipeline(0), "font");
	fontMaterial->setTexture(vkInstance, font->texture, 0);

	GPUTexture* shipTexture = nullptr;
	GPUTexture::getTexture("shipTexture", &shipTexture);
	material->setTexture(vkInstance, shipTexture, 0);

	GPUTexture* skyboxTexture = nullptr;
	GPUTexture::getTexture("skybox-top", &skyboxTexture);
	skyboxMaterial->setTexture(vkInstance, skyboxTexture, 0);
	GPUTexture::getTexture("skybox-front", &skyboxTexture);
	skyboxMaterial->setTexture(vkInstance, skyboxTexture, 1);
	GPUTexture::getTexture("skybox-right", &skyboxTexture);
	skyboxMaterial->setTexture(vkInstance, skyboxTexture, 2);
	GPUTexture::getTexture("skybox-back", &skyboxTexture);
	skyboxMaterial->setTexture(vkInstance, skyboxTexture, 3);
	GPUTexture::getTexture("skybox-left", &skyboxTexture);
	skyboxMaterial->setTexture(vkInstance, skyboxTexture, 4);
	GPUTexture::getTexture("skybox-bottom", &skyboxTexture);
	skyboxMaterial->setTexture(vkInstance, skyboxTexture, 5);
}

shml::vec3f inputDir{};
bool cursorLocked = true;

void Application::onWindowResized(GLFWwindow* window, int width, int height) {
	static Application* instance = getInstance();
	WindowResizeEvent we;
	we.window = window;
	we.width = width;
	we.height = height;

	SEND_WINDOW_EVENT(we);

	instance->renderer->notifyFramebufferResized();
	instance->window->Width = width;
	instance->window->Height = height;
}

void Application::onKeyDown(const Event<EKeyboardEvents>& keyPress) {
	KeyDownEvent evt = keyPress.toType<KeyDownEvent>();

	switch (evt.keyCode) {
		case GLFW_KEY_1:
			currentWorld = setupScene(0);
			break;
		case GLFW_KEY_2:
			currentWorld = setupScene(1);
			break;
		case GLFW_KEY_W:
			inputDir.z = -1;
			break;
		case GLFW_KEY_S:
			inputDir.z = 1;
			break;
		case GLFW_KEY_A:
			inputDir.x = 1;
			break;
		case GLFW_KEY_D:
			inputDir.x = -1;
			break;
		case GLFW_KEY_Q:
			inputDir.y = -1;
			break;
		case GLFW_KEY_E:
			inputDir.y = 1;
			break;
		case GLFW_KEY_LEFT_ALT:
			cursorLocked = !cursorLocked;
			break;
	}
}

void Application::onKeyUp(const Event<EKeyboardEvents>& keyPress) {
	KeyDownEvent evt = keyPress.toType<KeyDownEvent>();
	switch (evt.keyCode) {
	case GLFW_KEY_W:
	case GLFW_KEY_S:
		inputDir.z = 0;
		break;
	case GLFW_KEY_A:
	case GLFW_KEY_D:
		inputDir.x = 0;
		break;
	case GLFW_KEY_Q:
	case GLFW_KEY_E:
		inputDir.y = 0;
		break;
	}
}

World* Application::setupScene(int index) {
	static int currentScene = -1;
	if (currentScene == index) return currentWorld;

	if (index == 0) {
		static World* world0 = new World();
		static bool hasBeenCreated = false;
		if (hasBeenCreated) {
			currentScene = 0;
			return world0;
		}

		Material* fontMat;
		Material::find("font", &fontMat);
		TextMesh* fontMesh = TextMesh::generate(vkInstance, renderer, fontMat, window->Width, window->Height, FontAsset::find("default"),
			"Press Any Key to Start", { 1, 0, 600, 600 }, 1);

		world0->getRootNode()->addChild(fontMesh);
		hasBeenCreated = true;
		currentScene = 0;
		return world0;
	}
	else if (index == 1) {
		static World* world1 = new World();
		static bool hasBeenCreated = false;
		if (hasBeenCreated) {
			currentScene = 1;
			return world1;
		}
		Material* shipMat = nullptr;
		Material::find("default", &shipMat);

		Mesh* shipMesh = nullptr;
		Mesh* cube = nullptr;
		MeshLoader::loadOBJ("Assets/Mesh/ship.obj", &shipMesh);
		MeshLoader::loadOBJ("Assets/Mesh/cube.obj", &cube);

		RenderNode* entity = new RenderNode(renderer, shipMesh, shipMat);
		entity->setPosition({ 0, 0, -5 });
		entity->setRotation(shml::quat(0, 180.0, 0));
		world1->getRootNode()->addChild(entity);
		NodeMover* mover = new NodeMover(entity, entity);
		mover->speed = 0.02f;

		ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyDown, NodeMover::setVelocity, mover);
		ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyUp, NodeMover::zeroVelocity, mover);

		hasBeenCreated = true;
		currentScene = 1;
		return world1;
	}
	else return nullptr;
}

void Application::teardownCurrentScene() {
	
}

void Application::mainLoop() {
	ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyDown, Application::onKeyDown, this);
	ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyUp, Application::onKeyUp, this);
	
	Material* shipMat = nullptr;
	Material::find("default", &shipMat);
	Material* fontMat = nullptr;
	Material::find("font", &fontMat);

	currentWorld = setupScene(0);

	float time = 0;
	Camera mainCamera{};
	
	mainCamera.setup(window->Width, window->Height);

	glfwSetInputMode(window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (!glfwWindowShouldClose(window->GetWindow())) {
		time += 0.01f;
		glfwPollEvents();

		if (cursorLocked) {
			glfwSetInputMode(window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(window->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		
		inputDir = inputDir.normalized_safe() * 0.02f;

		currentWorld->executeCommands();
		currentWorld->getRootNode()->onDraw(currentWorld);
		currentWorld->getRootNode()->onUpdate(currentWorld, 0.02f);
		
		renderer->render(vkInstance, window->GetWindow(), &mainCamera);
	}
	
	vkDeviceWaitIdle(vkInstance->getDevice()->getLogicalDevice());
}

void Application::cleanup() {
	Material::cleanup(vkInstance);
	GPUTexture::cleanup(vkInstance);
	ShaderModule::teardown(vkInstance->getDevice()->getLogicalDevice());
	renderer->teardown(vkInstance->getDevice()->getLogicalDevice());
	delete renderer;
	vkInstance->teardown();
	delete vkInstance;
	window->teardown();
	
	glfwTerminate();
}

void uploadTextures(VulkanContext* vkInstance) {
	GPUTexture* texture;
	GPUTexture::getTexture("default", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("shipTexture", &texture);
	GPUTexture::loadGPU(vkInstance, texture);

	GPUTexture::getTexture("skybox-top", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("skybox-front", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("skybox-right", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("skybox-back", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("skybox-left", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
	GPUTexture::getTexture("skybox-bottom", &texture);
	GPUTexture::loadGPU(vkInstance, texture);
}
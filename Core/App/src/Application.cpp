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

#include "Core/Graphics/Mesh/Mesh.h"
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

void Application::onKeyDown(const Event<EKeyboardEvents>& keyPress) {
	KeyDownEvent evt = keyPress.toType<KeyDownEvent>();
	switch (evt.keyCode) {
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

void Application::mainLoop() {
	ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyDown, Application::onKeyDown, this);
	ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyUp, Application::onKeyUp, this);

	// Load Meshes
	Mesh* mesh = nullptr;
	Mesh* cube = nullptr;
	MeshLoader::loadOBJ("Assets/Mesh/ship.obj", &mesh);
	MeshLoader::loadOBJ("Assets/Mesh/cube.obj", &cube);

	// Create Player
	SceneNode* entity = new SceneNode();
	World::getWorld()->getRootNode()->addChild(entity);
	entity->setPosition({ 0, 0, -5 });
	entity->setRotation(shml::quat(0, 180.0, 0));

	// Load Materials and assign to render node.
	SceneNode* skyboxEntity = new SceneNode();
	Material* skyboxMat = nullptr;
	Material::find("skybox", &skyboxMat);
	RenderNode* skyboxRenderer = new RenderNode(World::getWorld()->getRootNode(), renderer, cube, skyboxMat);
	NodeMover* mover = new NodeMover(entity, entity);

	ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyDown, NodeMover::setVelocity, mover);
	ADD_KEYBOARD_EVENT_LISTENER(EKeyboardEvents::KeyUp, NodeMover::setVelocity, mover);

	Material* shipMat = nullptr;
	Material::find("default", &shipMat);
	Material* fontMat = nullptr;
	Material::find("font", &fontMat);

	RenderNode* rNode = new RenderNode(entity, renderer, mesh, shipMat);
	entity->addChild(rNode);
	TextMesh* fontMesh = TextMesh::generate(vkInstance, renderer, fontMat, window->Width, window->Height, FontAsset::find("default"),
		"Hello", {1, 0, 600, 600}, 8);

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
		World::getWorld()->getRootNode()->onUpdate(World::getWorld(), 0.02f);
		skyboxRenderer->setPosition((shml::vec3f)(mainCamera.transform.getRow(2)));
		World::getWorld()->moveEntity(entity, entity->getLocalPosition() + inputDir);
		World::getWorld()->executeCommands();
		
		const float* transform = entity->getTransform().getPointer();
		shipMat->setBuffer(vkInstance, 3, 0, transform, sizeof(shml::matrix4f));
		fontMesh->draw();
		renderer->render(vkInstance, window->GetWindow(), &mainCamera);
	}
	
	vkDeviceWaitIdle(vkInstance->getDevice()->getLogicalDevice());

	fontMesh->teardown(vkInstance);
	
	delete skyboxRenderer;
	delete rNode;
	delete fontMesh;
	delete entity;
	delete mesh;
	delete cube;
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
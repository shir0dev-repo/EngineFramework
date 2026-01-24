#pragma once

struct GLFWwindow;

struct AppWindow;
struct VulkanInstance;
struct GraphicsPipeline;

struct Application {
	int run();

	static void onWindowResized(GLFWwindow* window, int width, int height);
private:
	static Application* instance;
	bool initWindow();
	void initVulkan();
	void initGraphicsPipeline();
	void mainLoop();
	void cleanup();

	AppWindow* window = nullptr;
	VulkanInstance* vkInstance = nullptr;
	GraphicsPipeline* graphicsPipeline = nullptr;
};
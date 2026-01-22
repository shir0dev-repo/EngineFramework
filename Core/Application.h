#pragma once

struct AppWindow;
struct VulkanInstance;
struct GraphicsPipeline;

struct Application {
	int run();

private:
	bool initWindow();
	void initVulkan();
	void initGraphicsPipeline();
	void mainLoop();
	void cleanup();

	AppWindow* window = nullptr;
	VulkanInstance* vkInstance = nullptr;
	GraphicsPipeline* graphicsPipeline = nullptr;
};
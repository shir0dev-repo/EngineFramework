#pragma once

struct VulkanInstance;
struct AppWindow;

struct Application {
	int run();

private:
	bool initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

	AppWindow* window;
	VulkanInstance* vkInstance;
};
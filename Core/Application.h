#pragma once

struct VulkanInstance;
struct AppWindow;

struct Application {
	int run();

private:
	int initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

	AppWindow* window;
	VulkanInstance* vkInstance;
};
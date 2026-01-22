#pragma once

struct GLFWwindow;

struct VulkanInstance {
	struct VulkanValidator* validator = nullptr;
	struct VulkanDevice* device = nullptr;
	struct VkInstance_T* instance = nullptr;
	struct VkSurfaceKHR_T* surface = nullptr;
	struct VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;
	//struct VulkanSwapChain* swapChain = nullptr;

	bool setup(GLFWwindow* window);
	void teardown();
private:
	bool setupValidator();
	void setupInstance();
	void setupDebugMessenger();
	void setupSurface(GLFWwindow* window);
	void setupDevice(GLFWwindow* window);
	//void setupSwapchain(const struct VulkanDevice* vulkanDevice);

	void makeVkApplicationInfo(struct VkApplicationInfo& appInfo) const;
	void makeVkDebugMessengerCreateInfo(struct VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	void makeVkInstanceCreateInfo(const struct VkApplicationInfo& appInfo, struct VkInstanceCreateInfo& createInfo) const;
};
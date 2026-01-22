#pragma once

struct VulkanInstance {
	struct VulkanValidator* validator = nullptr;
	struct VulkanDevice* device = nullptr;
	struct VkInstance_T* instance = nullptr;
	struct VkSurfaceKHR_T* surface = nullptr;
	struct VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;

	bool setup(struct GLFWwindow* window);
	void teardown();
private:
	bool setupValidator();
	void setupInstance();
	void setupDebugMessenger();
	void setupSurface(struct GLFWwindow* window);
	void setupDevice(struct GLFWwindow* window);

	void makeVkApplicationInfo(struct VkApplicationInfo& appInfo) const;
	void makeVkDebugMessengerCreateInfo(struct VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	void makeVkInstanceCreateInfo(const struct VkApplicationInfo& appInfo, struct VkInstanceCreateInfo& createInfo) const;
};
#pragma once

struct VulkanInstance {
	VulkanInstance();
	struct VkInstance_T* instance = nullptr;
	struct VulkanValidator* validator = nullptr;
	struct VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;

	bool setup();
	void teardown();
private:
	void setupDebugMessenger();
	bool setupValidator();
	void setupInstance();

	struct VkApplicationInfo makeVkAppInfo() const;
	void makeVkDebugMessengerCreateInfo(struct VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	void makeVkInstanceCreateInfo(const struct VkApplicationInfo& appInfo, struct VkInstanceCreateInfo& createInfo) const;
};
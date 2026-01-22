#pragma once

typedef unsigned int uint32_t;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkInstance_T;
struct VkQueue_T;
struct VkSurfaceKHR_T;

struct GLFWwindow;

struct VulkanValidator;
struct VulkanSwapChain;
struct SwapChainSupportDetails;

struct VulkanDevice {
	VkPhysicalDevice_T* physicalDevice = nullptr;
	VkDevice_T* logicalDevice = nullptr;
	void setup(VkInstance_T* instance, const VulkanValidator& validator, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void teardown();

private:
	void pickPhysicalDevice(struct VkInstance_T* instance, VkSurfaceKHR_T* surface);
	void createLogicalDevice(const VulkanValidator& validator, VkSurfaceKHR_T* surface);
	
	VkQueue_T* graphicsQueue = nullptr;
	VkQueue_T* presentQueue = nullptr;
};
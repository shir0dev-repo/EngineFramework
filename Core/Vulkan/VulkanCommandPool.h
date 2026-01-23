#pragma once

struct VulkanSwapChain;

struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkSurfaceKHR_T;
struct VulkanDevice;

struct VulkanCommandPool {
	void setup(const VulkanDevice* const device, VkSurfaceKHR_T* surface);
	void teardown(VkDevice_T* logicalDevice);
	
	VkCommandBuffer_T* const getCommandBuffer() const { return commandBuffer; }

	void resetCommandBuffer();
	void beginRecordingCommandBuffer();
	void finishRecordingCommandBuffer();
private:
	void createCommandPool(const VulkanDevice* const device, VkSurfaceKHR_T* surface);
	void createCommandBuffer(VkDevice_T* logicalDevice);

	VkCommandPool_T* commandPool = nullptr;
	VkCommandBuffer_T* commandBuffer = nullptr;
};
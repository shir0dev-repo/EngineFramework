#pragma once

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint64_t VkDeviceSize;
typedef uint32_t VkFlags;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;

struct VkBufferCreateInfo;
struct VkBuffer_T;
struct VkDevice_T;
struct VkDeviceMemory_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;

struct VulkanDevice;
struct VulkanInstance;

struct GPUBuffer {
	VkBuffer_T* vkBuffer = nullptr;
	VkDeviceMemory_T* vkMemory = nullptr;
	
	VkBufferUsageFlags usageFlags;
	VkMemoryPropertyFlags memoryProperties;
	
	uint32_t bufferSize = 0;

	static GPUBuffer* create(const VulkanInstance* const instance, uint32_t sizeInBytes, VkBufferUsageFlags usage, const void* data = nullptr);
	void dispose(VkDevice_T* logicalDevice);

	void bind(VkCommandBuffer_T* commandBuffer);
	void bufferData(const VulkanInstance* const instance, const void* data, uint32_t sizeInBytes, uint32_t offset = 0);
	void unbind();
};
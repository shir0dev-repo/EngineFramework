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
struct VkCommandBuffer_T;

struct VulkanDevice;

struct GPUBuffer {
	uint32_t bufferSize = 0;

	static GPUBuffer* create(const VulkanDevice* const device, uint32_t sizeInBytes, VkBufferUsageFlags usage, const void* data = nullptr);
	static void copyBuffer(const VulkanDevice* const device, VkBuffer_T* src, VkBuffer_T* dst, VkDeviceSize size, uint32_t dstOffset = 0);

	void bind(VkCommandBuffer_T* commandBuffer);
	void bufferData(const VulkanDevice* const device, const void* data, uint32_t sizeInBytes, uint32_t offset = 0);
	void updateGPU(VkDevice_T* logicalDevice);
	void unbind();

	void dispose(VkDevice_T* logicalDevice);

private:
	static void createBuffer(const VulkanDevice* const device, VkDeviceSize sizeInBytes, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer_T*& buffer, VkDeviceMemory_T*& bufferMemory);
	
	void allocateGPU(const VulkanDevice* const logicalDevice, VkMemoryPropertyFlags properties, uint32_t bufferOffset = 0);

	VkBufferUsageFlags usageFlags;
	VkMemoryPropertyFlags memoryProperties;

	VkBuffer_T* vkBuffer = nullptr;
	VkDeviceMemory_T* vkMemory = nullptr;
};
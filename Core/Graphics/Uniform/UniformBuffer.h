#pragma once

typedef unsigned int uint32_t;
typedef uint32_t VkShaderStageFlags;
typedef uint32_t VkBufferUsageFlags;
typedef uint32_t VkMemoryPropertyFlags;

struct VkDevice_T;
struct VkBuffer_T;
struct VkDeviceMemory_T;

struct VulkanContext;

struct UniformBuffer {
	static UniformBuffer* create(const VulkanContext* const instance, uint32_t allocationSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryUsage);
	static void dispose(UniformBuffer*& buffer, VkDevice_T* logicalDevice);

	VkBuffer_T* vkBuffer;
	VkDeviceMemory_T* vkMemory;

	uint32_t binding = 0;
};
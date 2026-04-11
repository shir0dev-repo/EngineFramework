#pragma once

typedef unsigned long long VkDeviceSize;
typedef unsigned int uint32_t;
typedef uint32_t VkBufferUsageFlags;
typedef uint32_t VkMemoryPropertyFlags;

struct VkPhysicalDevice_T;
struct VkBuffer_T;
struct VkDeviceMemory_T;

struct VulkanContext;
struct GPUTexture;

struct BufferUtils {
	static uint32_t getMemoryType(VkPhysicalDevice_T* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void createBuffer(const VulkanContext* const instance, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryUsage,
		VkBuffer_T** buffer, VkDeviceMemory_T** memory);
	static void copyBuffer(const VulkanContext* const instance, VkBuffer_T* src, VkBuffer_T* dst, VkDeviceSize size, uint32_t dstOffset);


	static void copyToImage(const VulkanContext* const instance, VkBuffer_T* buffer, GPUTexture* texture);
};
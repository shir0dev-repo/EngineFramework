#pragma once

typedef unsigned long long VkDeviceSize;
typedef unsigned int uint32_t;
typedef uint32_t VkBufferUsageFlags;
typedef uint32_t VkMemoryPropertyFlags;

struct VkPhysicalDevice_T;
struct VkBuffer_T;
struct VkDeviceMemory_T;

struct VulkanDevice;
struct VulkanInstance;
struct GPUTexture;

struct BufferUtils {
	static uint32_t getMemoryType(VkPhysicalDevice_T* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void createBuffer(const VulkanInstance* const instance, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryUsage,
		VkBuffer_T** buffer, VkDeviceMemory_T** memory);
	static void copyBuffer(const VulkanInstance* const instance, VkBuffer_T* src, VkBuffer_T* dst, VkDeviceSize size, uint32_t dstOffset);


	static void copyToImage(const VulkanInstance* const instance, VkBuffer_T* buffer, GPUTexture* texture);
};
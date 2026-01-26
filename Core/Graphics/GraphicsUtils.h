#pragma once

struct VkCommandPool_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;

struct GraphicsUtils {
	static bool GetCurrentCommandPool(VkCommandPool_T** commandPool);
	static bool GetPhysicalDevice(VkPhysicalDevice_T*& physicalDevice);
	static bool GetLogicalDevice(VkDevice_T*& logicalDevice);
};
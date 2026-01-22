#pragma once

typedef unsigned int uint32_t;

struct VkPhysicalDevice_T;
struct VkSurfaceKHR_T;


struct QueueFamilyIndices {
	struct QueueFamilyIndex {
		uint32_t index;
		bool exists;
	};

	QueueFamilyIndex graphicsFamily = { 0, false };
	QueueFamilyIndex presentFamily = { 0, false };

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface);
	bool isComplete() const;
};
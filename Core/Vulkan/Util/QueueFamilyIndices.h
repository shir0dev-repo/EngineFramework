#pragma once

typedef unsigned int uint32_t;

struct VkPhysicalDevice_T;
struct VkSurfaceKHR_T;

/// <summary>Helper object for finding the indices of a device's queue family.</summary>
struct QueueFamilyIndices {
	/// <summary>Tuple of family index and whether it has a value or not.</summary>
	struct QueueFamilyIndex {
		uint32_t index;
		bool exists;
	};
	/// <summary>Queue family's graphics index.</summary>
	QueueFamilyIndex graphicsFamily = { 0, false };
	/// <summary>Queue family's present index.</summary>
	QueueFamilyIndex presentFamily = { 0, false };
	/// <summary>Finds a physical device's available queue families.</summary>
	/// <param name="device">The physical device to query.</param>
	/// <param name="surface">The surface being rendered to.</param>
	/// <param name="outQueueFamilyIndices">The resulting QueueFamilyIndices.</param>
	static void findQueueFamilies(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, QueueFamilyIndices& outQueueFamilyIndices);
	/// <summary>If the QueueFamilyIndices has both a graphics index and present index.</summary>
	bool isComplete() const;
};
#pragma once

struct VkImage_T;
struct VkDeviceMemory_T;
struct VkImageView_T;

struct DepthBuffer {
	VkImage_T* depthImage = nullptr;
	VkDeviceMemory_T* depthMemory = nullptr;
	VkImageView_T* depthImageView = nullptr;
};
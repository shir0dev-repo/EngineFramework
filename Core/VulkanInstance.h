#pragma once

struct VkInstance_T;

struct VulkanInstance {
	VulkanInstance();
	~VulkanInstance();

	VkInstance_T* instance;
};
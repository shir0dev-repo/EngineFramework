#include "../GraphicsUtils.h"
#include "../../Vulkan/VulkanDevice.h"
#include "../../Vulkan/VulkanInstance.h"
#include "../GraphicsPipeline.h"

#include <vulkan/vulkan.h>

bool GraphicsUtils::GetCurrentCommandPool(VkCommandPool_T** commandPool) {
	static VkCommandPool_T* cmdPool = nullptr;
	if (cmdPool == nullptr) {
		cmdPool = GraphicsPipeline::getInstance()->getCommandPool();
	}

	*commandPool = cmdPool;
	return *commandPool != nullptr;
}

bool GraphicsUtils::GetPhysicalDevice(VkPhysicalDevice_T*& physicalDevice) {
	static VkPhysicalDevice_T* device = nullptr;
	if (device == nullptr) {
		device = VulkanInstance::getInstance()->device->physicalDevice;
	}

	physicalDevice = device;
	return physicalDevice != nullptr;
}

bool GraphicsUtils::GetLogicalDevice(VkDevice_T*& logicalDevice) {
	static VkDevice_T* device = nullptr;
	if (device == nullptr) {
		device = VulkanInstance::getInstance()->device->logicalDevice;
	}

	logicalDevice = device;
	return logicalDevice != nullptr;
}

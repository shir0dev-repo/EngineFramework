#include "../BufferUtils.h"
#include "../../Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>
#include <iostream>

uint32_t BufferUtils::getMemoryType(VkPhysicalDevice_T* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProps = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

void BufferUtils::createBuffer(const VulkanDevice* const device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryUsage, VkBuffer_T** buffer, VkDeviceMemory_T** memory) {
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	if (vkCreateBuffer(device->logicalDevice, &createInfo, nullptr, buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}
	
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(device->logicalDevice, *buffer, &memRequirements);
	
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryType(device->physicalDevice, memRequirements.memoryTypeBits, memoryUsage);
	
	if (vkAllocateMemory(device->logicalDevice, &allocInfo, nullptr, memory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}
	
	vkBindBufferMemory(device->logicalDevice, *buffer, *memory, 0);
}
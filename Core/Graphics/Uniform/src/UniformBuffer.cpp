#include "../UniformBuffer.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>
#include <iostream>

static uint32_t getMemoryType(VkPhysicalDevice_T* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProps = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

UniformBuffer* UniformBuffer::create(const VulkanContext* const instance, uint32_t allocationSize,
	VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryUsage) {
	
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = allocationSize;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	UniformBuffer* buffer = new UniformBuffer();
	if (vkCreateBuffer(instance->getDevice()->getLogicalDevice(), &createInfo, nullptr, &buffer->vkBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(instance->getDevice()->getLogicalDevice(), buffer->vkBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryType(instance->getDevice()->getPhysicalDevice(), memRequirements.memoryTypeBits, memoryUsage);

	if (vkAllocateMemory(instance->getDevice()->getLogicalDevice(), &allocInfo, nullptr, &buffer->vkMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(instance->getDevice()->getLogicalDevice(), buffer->vkBuffer, buffer->vkMemory, 0);
	return buffer;
}

void UniformBuffer::dispose(UniformBuffer*& buffer, VkDevice_T* logicalDevice) {
	vkDestroyBuffer(logicalDevice, buffer->vkBuffer, nullptr);
	vkFreeMemory(logicalDevice, buffer->vkMemory, nullptr);
	
	delete buffer;
	buffer = nullptr;
}
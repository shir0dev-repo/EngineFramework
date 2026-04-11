#include "Core/Graphics/Buffer/BufferUtils.h"

#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"
#include "Core/Graphics/Texture/GPUTexture.h"

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

void BufferUtils::createBuffer(const VulkanContext* const instance, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryUsage, VkBuffer_T** buffer, VkDeviceMemory_T** memory) {
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	if (vkCreateBuffer(instance->getDevice()->getLogicalDevice(), &createInfo, nullptr, buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}
	
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(instance->getDevice()->getLogicalDevice(), *buffer, &memRequirements);
	
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryType(instance->getDevice()->getPhysicalDevice(), memRequirements.memoryTypeBits, memoryUsage);
	
	VkResult result = vkAllocateMemory(instance->getDevice()->getLogicalDevice(), &allocInfo, nullptr, memory);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}
	
	vkBindBufferMemory(instance->getDevice()->getLogicalDevice(), *buffer, *memory, 0);
}

void BufferUtils::copyBuffer(const VulkanContext* const instance, VkBuffer_T* src, VkBuffer_T* dst, VkDeviceSize size, uint32_t dstOffset) {
	VkCommandBuffer commandBuffer;
	instance->beginSingleUseCommandBuffer(&commandBuffer);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = dstOffset;

	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

	instance->endSingleUseCommandBuffer(commandBuffer);
}

void BufferUtils::copyToImage(const VulkanContext* const instance, VkBuffer_T* buffer, GPUTexture* texture) {
	VkCommandBuffer commandBuffer;
	instance->beginSingleUseCommandBuffer(&commandBuffer);
	
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	VkImageSubresourceRange subresourceRange = texture->imageViewInfo->subresourceRange;
	region.imageSubresource.aspectMask = subresourceRange.aspectMask;
	region.imageSubresource.mipLevel = subresourceRange.baseMipLevel;
	region.imageSubresource.baseArrayLayer = subresourceRange.baseArrayLayer;
	region.imageSubresource.layerCount = subresourceRange.layerCount;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { texture->width, texture->height, 1 };

	vkCmdCopyBufferToImage(commandBuffer,
		buffer,
		texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	instance->endSingleUseCommandBuffer(commandBuffer);
}
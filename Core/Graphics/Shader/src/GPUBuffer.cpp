#include "../GPUBuffer.h"
#include "../../../Vulkan/VulkanDevice.h"
#include "../../GraphicsUtils.h"

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

void GPUBuffer::createBuffer(const VulkanDevice* const device, VkDeviceSize sizeInBytes, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkBuffer_T*& buffer, VkDeviceMemory_T*& bufferMemory) {

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = sizeInBytes;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device->logicalDevice, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(device->logicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryType(device->physicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device->logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device->logicalDevice, buffer, bufferMemory, 0);
}

void GPUBuffer::copyBuffer(const VulkanDevice* const device, VkCommandPool_T* commandPool, VkBuffer_T* src, VkBuffer_T* dst, VkDeviceSize size, uint32_t dstOffset) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer_T* cmdBuffer;
	vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, &cmdBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = dstOffset;

	vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyRegion);
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->graphicsQueue);
	vkFreeCommandBuffers(device->logicalDevice, commandPool, 1, &cmdBuffer);
}

GPUBuffer* GPUBuffer::create(const VulkanDevice* const device, VkCommandPool_T* commandPool, uint32_t sizeInBytes, VkBufferUsageFlags usage, const void* data) {
	if (sizeInBytes <= 0 || data == nullptr) {
		return nullptr;
	}

	GPUBuffer* buffer = new GPUBuffer();
	buffer->usageFlags = usage;
	buffer->memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buffer->bufferSize = sizeInBytes;

	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(device, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, memProps, buffer->vkBuffer, buffer->vkMemory);
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(device, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProperties, stagingBuffer, stagingMemory);
	
	void* mappedData = nullptr;
	vkMapMemory(device->logicalDevice, stagingMemory, 0, sizeInBytes, 0, &mappedData);
	memcpy(mappedData, data, sizeInBytes);
	vkUnmapMemory(device->logicalDevice, stagingMemory);

	copyBuffer(device, commandPool, stagingBuffer, buffer->vkBuffer, sizeInBytes);
	vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);

	return buffer;
}

void GPUBuffer::bufferData(const VulkanDevice* const device, VkCommandPool_T* commandPool, const void* data, uint32_t sizeInBytes, uint32_t offset) {
	if (sizeInBytes <= 0) {
		throw std::runtime_error("Cannot buffer zero bytes of data!");
	}
	else if (offset + sizeInBytes > this->bufferSize) {
		throw std::runtime_error("Cannot buffer data! Size + Offset would result in buffer overrun.");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(device, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProperties, stagingBuffer, stagingMemory);

	void* mappedData;
	vkMapMemory(device->logicalDevice, stagingMemory, 0, sizeInBytes, 0, &mappedData);
	memcpy(mappedData, data, sizeInBytes);
	vkUnmapMemory(device->logicalDevice, stagingMemory);

	copyBuffer(device, commandPool, stagingBuffer, this->vkBuffer, sizeInBytes, offset);
}

void GPUBuffer::allocateGPU(const VulkanDevice* const device, VkMemoryPropertyFlags properties, uint32_t bufferOffset) {
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(device->logicalDevice, this->vkBuffer, &memRequirements);

	uint32_t memoryTypeIndex = getMemoryType(device->physicalDevice, memRequirements.memoryTypeBits, properties);
	
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	if (vkAllocateMemory(device->logicalDevice, &allocInfo, nullptr, &this->vkMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate GPU buffer memory!");
	}

	vkBindBufferMemory(device->logicalDevice, vkBuffer, vkMemory, bufferOffset);
	updateGPU(device->logicalDevice);
}

void GPUBuffer::updateGPU(VkDevice_T* logicalDevice) {
	void* mapped = nullptr;
	vkMapMemory(logicalDevice, vkMemory, 0, bufferSize, 0, &mapped);
	//memcpy(mapped, data, bufferSize);
	vkUnmapMemory(logicalDevice, vkMemory);
}

void GPUBuffer::bind(VkCommandBuffer_T* commandBuffer) {
	if ((usageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) != 0) {
		VkBuffer buffer[] = { vkBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffer, offsets);
	}
	else if ((usageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) != 0) {
		vkCmdBindIndexBuffer(commandBuffer, vkBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}

void GPUBuffer::unbind() {
	
}

void GPUBuffer::dispose(VkDevice_T* logicalDevice) {
	if (vkBuffer != nullptr) {
		vkDestroyBuffer(logicalDevice, vkBuffer, nullptr);
		vkBuffer = nullptr;
	}
	if (vkMemory != nullptr) {
		vkFreeMemory(logicalDevice, vkMemory, nullptr);
		vkMemory = nullptr;
	}
}
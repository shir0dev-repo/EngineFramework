#include "../GPUBuffer.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"
#include "Core/Graphics/GraphicsUtils.h"
#include "Core/Graphics/Buffer/BufferUtils.h"

#include <vulkan/vulkan.h>
#include <iostream>

GPUBuffer* GPUBuffer::create(const VulkanContext* const instance, uint32_t sizeInBytes, VkBufferUsageFlags usage, const void* data) {
	
	if (sizeInBytes <= 0) {
		return nullptr;
	}

	GPUBuffer* buffer = new GPUBuffer();
	buffer->usageFlags = usage;
	buffer->memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	buffer->bufferSize = sizeInBytes;

	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	BufferUtils::createBuffer(instance, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, memProps, &buffer->vkBuffer, &buffer->vkMemory);
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	BufferUtils::createBuffer(instance, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProperties, &stagingBuffer, &stagingMemory);
	
	if (data != nullptr) {
		void* mappedData = nullptr;
		vkMapMemory(instance->getDevice()->getLogicalDevice(), stagingMemory, 0, sizeInBytes, 0, &mappedData);
		memcpy(mappedData, data, sizeInBytes);
		vkUnmapMemory(instance->getDevice()->getLogicalDevice(), stagingMemory);
	}

	BufferUtils::copyBuffer(instance, stagingBuffer, buffer->vkBuffer, sizeInBytes, 0);

	vkDestroyBuffer(instance->getDevice()->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(instance->getDevice()->getLogicalDevice(), stagingMemory, nullptr);

	return buffer;
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

void GPUBuffer::bufferData(const VulkanContext* const instance, const void* data, uint32_t sizeInBytes, uint32_t offset) {
	if (sizeInBytes <= 0) {
		throw std::runtime_error("Cannot buffer zero bytes of data!");
	}
	else if (offset + sizeInBytes > this->bufferSize) {
		throw std::runtime_error("Cannot buffer data! Size + Offset would result in buffer overrun.");
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	BufferUtils::createBuffer(instance, sizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProperties, &stagingBuffer, &stagingMemory);

	void* mappedData;
	vkMapMemory(instance->getDevice()->getLogicalDevice(), stagingMemory, 0, sizeInBytes, 0, &mappedData);
	memcpy(mappedData, data, sizeInBytes);
	vkUnmapMemory(instance->getDevice()->getLogicalDevice(), stagingMemory);
	
	BufferUtils::copyBuffer(instance, stagingBuffer, this->vkBuffer, sizeInBytes, offset);
	vkDestroyBuffer(instance->getDevice()->getLogicalDevice(), stagingBuffer, nullptr);
	vkFreeMemory(instance->getDevice()->getLogicalDevice(), stagingMemory, nullptr);
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
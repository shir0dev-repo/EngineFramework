#include "../VulkanCommandPool.h"
#include "../Util/QueueFamilyIndices.h"
#include "../VulkanDevice.h"

#include <vulkan/vulkan.h>
#include <iostream>

void VulkanCommandPool::setup(const VulkanDevice* const device, VkSurfaceKHR_T* surface) {
	createCommandPool(device, surface);
	createCommandBuffer(device->logicalDevice);
}

void VulkanCommandPool::createCommandPool(const VulkanDevice* const device, VkSurfaceKHR_T* surface) {
	const QueueFamilyIndices* const indices = device->getQueueFamilyIndices();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices->graphicsFamily.index;

	if (vkCreateCommandPool(device->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}
void VulkanCommandPool::createCommandBuffer(VkDevice_T* logicalDevice) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &this->commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void VulkanCommandPool::resetCommandBuffer() {
	vkResetCommandBuffer(commandBuffer, 0);
}
void VulkanCommandPool::beginRecordingCommandBuffer() {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
}

void VulkanCommandPool::finishRecordingCommandBuffer() {}

void VulkanCommandPool::teardown(VkDevice_T* logicalDevice) {
	if (commandPool != nullptr) {
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		commandPool = nullptr;
	}
}
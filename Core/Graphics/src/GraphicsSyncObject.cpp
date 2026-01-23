#include "../GraphicsSyncObject.h"

#include <vulkan/vulkan.h>
#include <iostream>

void GraphicsSyncObject::setup(VkDevice_T* logicalDevice) {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult imageAvailableCreateResult = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
	VkResult renderFinishedCreateResult = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
	VkResult fenceCreateResult = vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFence);
	
	if ((imageAvailableCreateResult | renderFinishedCreateResult | fenceCreateResult) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create semaphores!");
	}
}

void GraphicsSyncObject::wait(VkDevice_T* logicalDevice) {
	vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(logicalDevice, 1, &inFlightFence);
}
void GraphicsSyncObject::reset(VkDevice_T* logicalDevice) {
	vkResetFences(logicalDevice, 1, &inFlightFence);
}

void GraphicsSyncObject::teardown(VkDevice_T* logicalDevice) {
	vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, nullptr);
	vkDestroyFence(logicalDevice, inFlightFence, nullptr);
}
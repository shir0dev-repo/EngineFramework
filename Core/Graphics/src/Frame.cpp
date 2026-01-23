#include "../Frame.h"
#include "../GraphicsSyncObject.h"

#include <vulkan/vulkan.h>

void Frame::setup(VkDevice_T* logicalDevice, VkCommandBuffer_T*& commandBuffer) {
	syncObject = new GraphicsSyncObject();
	syncObject->setup(logicalDevice);

	this->commandBuffer = commandBuffer;
}

void Frame::teardown(VkDevice_T* logicalDevice) {
	syncObject->teardown(logicalDevice);
	vkDestroyFramebuffer(logicalDevice, frameBuffer, nullptr);
}
#include "../Frame.h"
#include "../GraphicsSyncObject.h"

#include <vulkan/vulkan.h>
#include <iostream>

void Frame::setupBuffer(VkDevice_T* logicalDevice, VkCommandBuffer_T*& commandBuffer, const VkExtent2D* const extent, VkRenderPass_T* renderPass) {
	syncObject = new GraphicsSyncObject();
	syncObject->setup(logicalDevice);
	
	this->commandBuffer = commandBuffer;

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &imageView;
	framebufferInfo.width = extent->width;
	framebufferInfo.height = extent->height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &this->frameBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create framebuffer!");
	}
}

void Frame::setupImageView(VkDevice_T* logicalDevice, const VkFormat& surfaceFormat, VkImage_T* swapchainImage) {
	if (this->image == nullptr || this->image != swapchainImage)
		this->image = swapchainImage;

	VkImageViewCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = this->image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = surfaceFormat;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &this->imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image view!");
	}
}

void Frame::teardown(VkDevice_T* logicalDevice, bool isFinalTeardown) {
	if (isFinalTeardown)
		syncObject->teardown(logicalDevice);

	vkDestroyFramebuffer(logicalDevice, this->frameBuffer, nullptr);
	frameBuffer = nullptr;

	vkDestroyImageView(logicalDevice, this->imageView, nullptr);
	imageView = nullptr;
}
#pragma once

typedef enum VkFormat;

struct VkCommandBuffer_T;
struct VkDevice_T;
struct VkFramebuffer_T;
struct VkImage_T;
struct VkImageView_T;
struct VkExtent2D;
struct VkRenderPass_T;

struct GraphicsSyncObject;

struct Frame {
	VkImage_T* image = nullptr;
	VkImageView_T* imageView = nullptr;

	VkCommandBuffer_T* commandBuffer = nullptr;
	GraphicsSyncObject* syncObject = nullptr;
	VkFramebuffer_T* frameBuffer = nullptr;

	void setupSyncs(VkDevice_T* logicalDevice);
	void setupImageView(VkDevice_T* logicalDevice, const VkFormat& surfaceFormat, VkImage_T* swapchainImage);
	void setupBuffer(VkDevice_T* logicalDevice, const VkExtent2D* const extent, VkRenderPass_T* renderPass);

	void teardown(VkDevice_T* logicalDevice, bool isFinalTeardown);
};
#pragma once

typedef enum VkFormat;

struct VkCommandBuffer_T;
struct VkDevice_T;
struct VkDeviceMemory_T;
struct VkBuffer_T;
struct VkFramebuffer_T;
struct VkImage_T;
struct VkImageView_T;
struct VkExtent2D;
struct VkRenderPass_T;
struct VkBuffer_T;
struct GraphicsSyncObject;
struct VulkanDevice;
struct MatrixBufferObject;

struct Frame {
	VkBuffer_T* uniformBuffer = nullptr;
	VkDeviceMemory_T* uniformMemory = nullptr;
	void* uniformBufferMapped = nullptr;

	VkImage_T* image = nullptr;
	VkImageView_T* imageView = nullptr;

	VkCommandBuffer_T* commandBuffer = nullptr;
	GraphicsSyncObject* syncObject = nullptr;
	VkFramebuffer_T* frameBuffer = nullptr;

	void setupSyncs(VkDevice_T* logicalDevice);
	void setupImageView(VkDevice_T* logicalDevice, const VkFormat& surfaceFormat, VkImage_T* swapchainImage);
	void setupBuffer(const VulkanDevice* const device, const VkExtent2D* const extent, VkRenderPass_T* renderPass);

	void updateUniforms(const MatrixBufferObject* viewProjMatrices);

	void teardown(VkDevice_T* logicalDevice, bool isFinalTeardown);
};
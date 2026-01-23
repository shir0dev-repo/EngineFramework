#pragma once

struct VkCommandBuffer_T;
struct VkDevice_T;
struct VkFramebuffer_T;

struct GraphicsSyncObject;

struct Frame {
	VkCommandBuffer_T* commandBuffer = nullptr;
	GraphicsSyncObject* syncObject = nullptr;
	VkFramebuffer_T* frameBuffer = nullptr;

	void setup(VkDevice_T* logicalDevice, VkCommandBuffer_T*& commandBuffer);
	void teardown(VkDevice_T*);

	void beginCommandBuffer();
};
#pragma once

struct VkSemaphore_T;
struct VkFence_T;
struct VkDevice_T;

struct GraphicsSyncObject {
	VkSemaphore_T* imageAvailableSemaphore;
	VkSemaphore_T* renderFinishedSemaphore;
	VkFence_T * inFlightFence;

	void setup(VkDevice_T* logicalDevice);
	void teardown(VkDevice_T* logicalDevice);

	void wait(VkDevice_T* logicalDevice);
	void reset(VkDevice_T* logicalDevice);
};
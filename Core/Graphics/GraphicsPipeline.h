#pragma once

typedef unsigned int uint32_t;

struct VkDevice_T;
struct VulkanSwapChain;
struct VkPipelineLayout_T;

struct GraphicsPipeline {
	void setup(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);
	void teardown(VkDevice_T* logicalDevice);

	static bool readFile(const char* filePath, char*& outFileContents, uint32_t& fileSize);

private:
	VkPipelineLayout_T** pipelineLayout = nullptr;
};
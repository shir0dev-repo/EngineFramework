#pragma once

typedef unsigned int uint32_t;

struct VkDevice_T;
struct VkRenderPass_T;
struct VkPipelineLayout_T;
struct VkPipeline_T;
struct VkFramebuffer_T;
struct VkSurfaceKHR_T;
struct VkExtent2D;
struct VkCommandPool_T;
struct VkCommandBuffer_T;

struct VulkanCommandPool;
struct VulkanDevice;
struct VulkanSwapChain;
struct GraphicsSyncObject;
struct Frame;

struct GraphicsPipeline {
	const uint32_t MAX_FRAMEBUFFERS = 3;
	static bool readFile(const char* filePath, char*& outFileContents, uint32_t& fileSize);
	
	void setup(const VulkanDevice* const device, const VulkanSwapChain* const swapChain, VkSurfaceKHR_T* surface);
	void teardown(VkDevice_T* logicalDevice);

	void beginRenderPass(VkCommandBuffer_T* commandBuffer, const VulkanSwapChain* const swapChain);
	void addRenderCommmand(VkCommandBuffer_T* commandBuffer);
	void finalizeRenderPass(VkCommandBuffer_T* commandBuffer);

	void render(const VulkanDevice* const device, const VulkanSwapChain* const swapChain);
private:
	void setupRenderPass(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);
	void setupPipelineLayout(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);
	void setupCommandBuffers(const VulkanDevice* const device, VkSurfaceKHR_T* surface);
	void setupFramebuffers(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);

	void initViewportScissor(VkCommandBuffer_T* commandBuffer, const VkExtent2D* extent);

	VkRenderPass_T* renderPass = nullptr;
	VkPipelineLayout_T* pipelineLayout = nullptr;
	VkPipeline_T* pipeline = nullptr;

	uint32_t currentFramebufferIndex = 0;
	uint32_t framebufferCount = 0;

	VkCommandPool_T* commandPool = nullptr;
	Frame** framebufferNEW = nullptr;
};
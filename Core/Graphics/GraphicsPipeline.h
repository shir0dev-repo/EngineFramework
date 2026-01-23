#pragma once

typedef unsigned int uint32_t;

struct VkDevice_T;
struct VkRenderPass_T;
struct VkPipelineLayout_T;
struct VkPipeline_T;
struct VkFramebuffer_T;
struct VkSurfaceKHR_T;
struct VkExtent2D;
struct VkCommandBuffer_T;

struct VulkanCommandPool;
struct VulkanDevice;
struct VulkanSwapChain;
struct GraphicsSyncObject;

struct GraphicsPipeline {
	static bool readFile(const char* filePath, char*& outFileContents, uint32_t& fileSize);
	
	void setup(const VulkanDevice* const device, const VulkanSwapChain* const swapChain, VkSurfaceKHR_T* surface);
	void teardown(VkDevice_T* logicalDevice);

	void beginRenderPass(const VulkanSwapChain* const swapChain);
	void addRenderCommmand();
	void finalizeRenderPass(const VulkanSwapChain* const swapChain);

	void render(const VulkanDevice* const device, const VulkanSwapChain* const swapChain);
private:
	void setupRenderPass(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);
	void setupPipelineLayout(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);
	void setupFramebuffers(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain);
	void setupCommandPool(const VulkanDevice* const device, VkSurfaceKHR_T* surface);
	void setupSyncObject(VkDevice_T* logicalDevice);

	void initViewportScissor(VkCommandBuffer_T* commandBuffer, const VkExtent2D* extent);

	VkRenderPass_T* renderPass = nullptr;
	VkPipelineLayout_T* pipelineLayout = nullptr;
	VkPipeline_T* pipeline = nullptr;

	uint32_t currentFramebufferIndex = 0;
	uint32_t framebufferCount = 0;
	VkFramebuffer_T** framebuffers = nullptr;

	VulkanCommandPool* commandPool = nullptr;

	GraphicsSyncObject* syncObject = nullptr;
};
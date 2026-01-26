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
struct VkBuffer_T;
struct VkCommandBuffer_T;

struct GLFWwindow;

struct VulkanCommandPool;
struct VulkanDevice;
struct VulkanSwapChain;
struct GraphicsSyncObject;
struct GPUBuffer;
struct Frame;

struct GraphicsPipeline {
	const uint32_t MAX_FRAMEBUFFERS = 3;
	static bool readFile(const char* filePath, char*& outFileContents, uint32_t& fileSize);
	
	static GraphicsPipeline* const getInstance();

	void setup(const VulkanDevice* const device, VulkanSwapChain* const swapChain, VkSurfaceKHR_T* surface);
	void teardown(VkDevice_T* logicalDevice);

	VkCommandPool_T* const getCommandPool() { return commandPool; }

	static void onWindowResized(GraphicsPipeline* pipelineInstance, GLFWwindow* window, int width, int height);
	void addRenderCommand(GPUBuffer* buffers, uint32_t bufferCount, uint32_t vertexCount, uint32_t indexCount);
	void render(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window);
private:
	static GraphicsPipeline* instance;

	void setupRenderPass(VkDevice_T* logicalDevice);
	void setupPipelineLayout(VkDevice_T* logicalDevice);
	void setupCommandPool(const VulkanDevice* const device, VkSurfaceKHR_T* surface);
	void setupVertexBuffer(const VulkanDevice* const device);
	void allocCommandBuffers(VkDevice_T* logicalDevice, const uint32_t& count, VkCommandBuffer_T**& outBuffers);
	void setupFramebuffers(VkDevice_T* logicalDevice, const uint32_t& bufferCount, VkCommandBuffer_T** commandBuffers);

	bool initFrame(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void beginCommandBuffer(VkCommandBuffer_T* commandBuffer);
	void beginRenderPass(VkCommandBuffer_T* commandBuffer);
	void iterateRenderCommands(VkCommandBuffer_T* commandBuffer);
	void addRenderCommmand(VkCommandBuffer_T* commandBuffer);
	void finishRenderPass(VkCommandBuffer_T* commandBuffer);
	void finishCommandBuffer(VkCommandBuffer_T* commandBuffer);

	void submitRender(const VulkanDevice* const device, Frame* currentFrame);
	void presentRender(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window);

	void initViewportScissor(VkCommandBuffer_T* commandBuffer, const VkExtent2D* extent);

	bool frameBufferResized = false;

	VkRenderPass_T* renderPass = nullptr;
	VkPipelineLayout_T* pipelineLayout = nullptr;
	VkPipeline_T* pipeline = nullptr;
	VulkanSwapChain* swapChain = nullptr;

	VkCommandPool_T* commandPool = nullptr;
	VkCommandBuffer_T* transferCommmandBuffer = nullptr;
};
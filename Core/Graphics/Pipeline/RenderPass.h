#pragma once

typedef unsigned int uint32_t;
typedef uint32_t VkBool32;

struct VkSurfaceKHR_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkRenderPass_T;
struct VkBuffer_T;
struct VkDeviceMemory_T;

struct GLFWwindow;

struct ShaderModule;
struct VulkanSwapChain;
struct VulkanInstance;
struct VulkanDevice;
struct GraphicsPipeline;
struct Frame;

template<typename T>
class linkedList;
typedef linkedList<GraphicsPipeline*> PipelineList_T;

struct RenderPass {
	void setup(VulkanInstance* vkInstance);
	void teardown();

	Frame* const getCurrentFrame() const;

	VkRenderPass_T* const getRenderPass() { return vkRenderPass; }
	VkCommandPool_T* const getCommandPool() { return commandPool; }

	GraphicsPipeline* const createPipeline(const VulkanDevice* const device, ShaderModule* vertex, ShaderModule* fragment, bool isOpaque = true);
	void removePipeline(const VulkanDevice* const device, GraphicsPipeline* pipeline);

	void onRender(VulkanInstance* const vulkanInstance, GLFWwindow* window);
private:
	void setupRenderPass(const VulkanDevice* const device);
	void setupCommandPool(VulkanInstance* vkInstance);
	void setupFramebuffers(const VulkanDevice* const device);

	bool beginFrame(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void beginCommandBuffer(VkCommandBuffer_T* commandBuffer);
	void beginPass(VkCommandBuffer_T* commandBuffer);
	void beginPipeline(VkCommandBuffer_T* commandBuffer, GraphicsPipeline* pipeline);
	void renderPipeline(VkCommandBuffer_T* commandBuffer, GraphicsPipeline* pipeline);
	void finalizePipeline(VkCommandBuffer_T* commandBuffer, GraphicsPipeline* pipeline);
	void finalizePass(VkCommandBuffer_T* commandBuffer);
	
	void submitRender(const VulkanDevice* const device, Frame* currentFrame);
	void presentRender(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window);
	
	VkBuffer_T* cameraUniformBuffer = nullptr;
	VkDeviceMemory_T* cameraUniformMemory = nullptr;

	VkBool32 frameBufferResized = 0;
	uint32_t currentFrameIndex = 0;
	Frame** framebuffers = nullptr;

	VulkanSwapChain* swapChain = nullptr;
	VkRenderPass_T* vkRenderPass = nullptr;
	PipelineList_T* graphicsPipelines = nullptr;
	VkCommandPool_T* commandPool = nullptr;
};
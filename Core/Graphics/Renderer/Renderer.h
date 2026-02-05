#pragma once

typedef unsigned int uint32_t;

struct VkDescriptorSetLayout_T;
struct VkDescriptorPool_T;
struct VkDescriptorSet_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkFramebuffer_T;
struct VkSurfaceKHR_T;
struct VkRenderPass_T;
struct VkDevice_T;

struct GLFWwindow;

struct VulkanInstance;
struct VulkanDevice;
struct VulkanSwapChain;
struct RenderPass;
struct GraphicsPipeline;
struct GraphicsSyncObject;
struct Frame;
struct UniformBuffer;
struct PipelineShader;

template <typename T>
class linkedList;

struct Renderer {
	VkCommandPool_T* const getCurrentCommandPool() const;
	VkRenderPass_T* const getRenderPass() const;
	VkDescriptorSetLayout_T* const getGlobalDescriptorSetLayout() const;

	static Renderer* const getInstance();

	void addPipeline(PipelineShader* shader);
	GraphicsPipeline* const getPipeline(const PipelineShader* shader);
	void removePipeline(GraphicsPipeline* pipeline);

	void render(VulkanInstance* instance, GLFWwindow* window);

	void setup(VulkanInstance* vkInstance);
	void teardown(VkDevice_T* logicalDevice);

private:
	void setupRenderPass(const VulkanDevice* const device);
	void setupCommandPool(const VulkanDevice* const device);
	void setupCommandBuffers(const VulkanDevice* const device);
	void setupFramebuffers(const VulkanDevice* const device);
	void setupSyncs(const VulkanDevice* const device);
	void setupGlobalUniforms(const VulkanInstance* const instance);
	void setupGlobalDescriptorLayout(const VulkanInstance* const instance);
	void setupGlobalDescriptorPool(const VulkanInstance* const instance);
	void setupGlobalDescriptorSets(const VulkanInstance* const instance);

	bool beginFrame(const VulkanDevice* const device);
	void updateGlobalBuffer(const VulkanDevice* const device);
	void beginCommandBufferForCurrentFrame();
	void bindGlobalDescriptors(GraphicsPipeline* pipeline);
	void beginRenderPassForCurrentFrame();
	void initViewportScissorForCurrentFrame();
	void beginPipeline(GraphicsPipeline* pipeline);
	void executePipeline(GraphicsPipeline* pipeline);
	void finalizePipeline(GraphicsPipeline* pipeline);
	void finalizeRenderPassForCurrentFrame();
	void finalizeCommandBufferForCurrentFrame();

	void submitRender(const VulkanDevice* const device);
	bool presentRender(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window);

	linkedList<GraphicsPipeline*>* graphicsPipelines = nullptr;

	VulkanSwapChain* swapChain = nullptr;
	VkRenderPass_T* vkRenderPass = nullptr;

	VkFramebuffer_T** vkFramebuffers = nullptr;
	GraphicsSyncObject* syncObjects = nullptr;

	VkCommandPool_T* vkCommandPool = nullptr;
	VkCommandBuffer_T** vkCommandBuffers = nullptr;

	UniformBuffer** globalBuffers = nullptr;

	VkDescriptorSetLayout_T* globalDescriptorLayout = nullptr;
	VkDescriptorPool_T* globalDescriptorPool = nullptr;
	VkDescriptorSet_T** globalDescriptorSets = nullptr;
	void** mappedGlobalBuffers = nullptr;

	uint32_t numFrames = 0;
	uint32_t currentFrame = 0;
};
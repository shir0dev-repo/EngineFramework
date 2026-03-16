#pragma once

typedef unsigned int uint32_t;
typedef enum VkFormat;

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

struct Camera;
struct VulkanInstance;
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
	static const int GLOBAL_DESCRIPTOR_SET = 0;
	static const int PIPELINE_DESCRIPTOR_SET = 1;
	static const int MATERIAL_DESCRIPTOR_SET = 2;
	static const int INSTANCE_DESCRIPTOR_SET = 3;

	VkCommandPool_T* const getCurrentCommandPool() const;
	VkRenderPass_T* const getRenderPass() const;
	VkDescriptorSetLayout_T* const getGlobalDescriptorSetLayout() const;

	static Renderer* const getInstance();

	void addPipeline(PipelineShader* shader, bool transparent);
	GraphicsPipeline* const getPipeline(uint32_t index);
	GraphicsPipeline* const getTransparentPipeline(uint32_t index);

	void removePipeline(GraphicsPipeline* pipeline);

	void render(VulkanInstance* instance, GLFWwindow* window, Camera* camera);
	
	void notifyFramebufferResized();

	void setup(VulkanInstance* vkInstance);
	void teardown(VkDevice_T* logicalDevice);

private:
	void setupRenderPass(const VulkanInstance* const instance);
	void setupCommandPool(const VulkanInstance* const instance);
	void setupCommandBuffers(const VulkanInstance* const instance);
	void setupDepthBuffer(const VulkanInstance* const instance);
	void setupFramebuffers(const VulkanInstance* const instance);
	void setupSyncs(const VulkanInstance* const instance);
	void setupGlobalUniforms(const VulkanInstance* const instance);
	void setupGlobalDescriptorLayout(const VulkanInstance* const instance);
	void setupGlobalDescriptorPool(const VulkanInstance* const instance);
	void setupGlobalDescriptorSets(const VulkanInstance* const instance);

	bool beginFrame(const VulkanInstance* const instance);
	void updateGlobalBuffer(const VulkanInstance* const instance, Camera* camera);
	void beginCommandBufferForCurrentFrame();
	void bindGlobalDescriptors(GraphicsPipeline* pipeline);
	void beginRenderPassForCurrentFrame();
	void initViewportScissorForCurrentFrame();
	void beginPipeline(GraphicsPipeline* pipeline);
	void executePipeline(GraphicsPipeline* pipeline);
	void finalizePipeline(GraphicsPipeline* pipeline);
	void finalizeRenderPassForCurrentFrame();
	void finalizeCommandBufferForCurrentFrame();

	void submitRender(const VulkanInstance* const instance);
	bool presentRender(const VulkanInstance* const instance);

	void handleInvalidSwapchain(const VulkanInstance* const instance, GLFWwindow* window);

	linkedList<GraphicsPipeline*>* graphicsPipelines = nullptr;
	linkedList<GraphicsPipeline*>* transparentPipelines = nullptr;

	VulkanSwapChain* swapChain = nullptr;
	VkRenderPass_T* vkRenderPass = nullptr;
	VkRenderPass_T* transparentRenderPass = nullptr;

	VkFramebuffer_T** vkFramebuffers = nullptr;
	GraphicsSyncObject* syncObjects = nullptr;

	VkCommandPool_T* vkCommandPool = nullptr;
	VkCommandBuffer_T** vkCommandBuffers = nullptr;

	UniformBuffer** globalBuffers = nullptr;

	VkDescriptorSetLayout_T* globalDescriptorLayout = nullptr;
	VkDescriptorPool_T* globalDescriptorPool = nullptr;
	VkDescriptorSet_T** globalDescriptorSets = nullptr;
	void** mappedGlobalBuffers = nullptr;

	struct DepthBuffer* depthBuffer;

	uint32_t numFrames = 0;
	uint32_t currentFrame = 0;
	bool frameBufferResized = false;

	static VkFormat findDepthFormat(const VulkanInstance* const device);
};
#pragma once

typedef unsigned int uint32_t;

struct VkRenderPass_T;
struct VkPipeline_T;
struct VkPipelineLayout_T;
struct VkCommandBuffer_T;
struct VkDevice_T;
struct VkDescriptorSetLayout_T;
struct VkDescriptorPool_T;
struct VkDescriptorSet_T;

struct VkPipelineShaderStageCreateInfo;

struct Renderer;
struct VulkanInstance;
struct VulkanDevice;
struct MeshRenderer;

struct PipelineSummary;
struct ShaderModule;
struct RenderCommand;

template<typename T>
class linkedList;
typedef linkedList<RenderCommand*> RenderCommandList;

struct GraphicsPipeline {
	VkPipeline_T* const getPipeline();
	VkPipelineLayout_T* const getLayout() const;
	void setup(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment);
	void teardown(VkDevice_T* logicalDevice);

	void addRenderCommand(const MeshRenderer* const meshRenderer);
	void executeRenderCommands(VkCommandBuffer_T* commandBuffer);
private:
	static VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(const ShaderModule*& shader);
	void createPipeline(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment);
	void createDescriptorPool(const VulkanInstance* const instance);
	void createDescriptorSets(const VulkanInstance* const instance, VkDescriptorSetLayout_T** setLayouts, uint32_t layoutCount);
	void createUniformBuffers(const VulkanInstance* const instance);

	void updateDescriptorSets(const VulkanInstance* const instance);

	void cleanupRenderCommands();
	
	PipelineSummary* summary = nullptr;
	VkPipeline_T* vkPipeline = nullptr;
	VkPipelineLayout_T* vkLayout = nullptr;

	uint32_t numSetLayouts = 0;
	uint32_t numDescriptorSets = 0;

	VkDescriptorSetLayout_T** vkDescriptorSetLayouts = nullptr;
	VkDescriptorPool_T* vkDescriptorPool = nullptr;
	VkDescriptorSet_T*** vkDescriptorSets = nullptr;

	//UniformBuffer* uniformBuffers = nullptr;
	RenderCommandList* commandList = nullptr;
};
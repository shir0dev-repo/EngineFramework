#pragma once

#include "PipelineMaterialLayout.h"

typedef unsigned int uint32_t;

struct VkRenderPass_T;
struct VkPipeline_T;
struct VkPipelineLayout_T;
struct VkCommandBuffer_T;
struct VkDevice_T;
struct VkDescriptorSetLayout_T;
struct VkDescriptorSetLayoutBinding;
struct VkDescriptorPool_T;
struct VkDescriptorSet_T;

struct VkPipelineShaderStageCreateInfo;

struct Renderer;
struct VulkanInstance;
struct VulkanDevice;
struct MeshRenderer;
struct DescriptorHandle;

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

	void bindDescriptorSets(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame);

	const PipelineMaterialLayout* const getMaterialLayout() const { return &materialLayout; }

	void addRenderCommand(const MeshRenderer* const meshRenderer);
	void executeRenderCommands(VkCommandBuffer_T* commandBuffer);
private:
	static VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(const ShaderModule*& shader);
	void createPipelineSummary(const VulkanInstance* const instance, ShaderModule* vertex, ShaderModule* fragment);
	void createPipeline(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment);
	void createMaterialLayout(const VulkanInstance* const instance);

	void setupDescriptors(const VulkanInstance* const instance);
	void createDescriptorPool(const VulkanInstance* const instance);
	void createDescriptorSetLayout(const VulkanInstance* const instance, VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t layoutCount);
	void createDescriptorSets(const VulkanInstance* const instance);
	void updateDescriptorWrites(const VulkanInstance* const instance);

	void cleanupRenderCommands();
	
	PipelineSummary* summary = nullptr;
	VkPipeline_T* vkPipeline = nullptr;
	VkPipelineLayout_T* vkLayout = nullptr;

	uint32_t numDescriptorSets = 0;
	uint32_t numDescriptorCopies = 0;
	
	VkDescriptorPool_T* vkDescriptorPool = nullptr;
	VkDescriptorSetLayout_T* vkDescriptorLayout = nullptr;
	VkDescriptorSet_T*** vkDescriptorSets = nullptr;

	DescriptorHandle** descriptorHandles = nullptr;

	RenderCommandList* commandList = nullptr;

	PipelineMaterialLayout materialLayout;
};
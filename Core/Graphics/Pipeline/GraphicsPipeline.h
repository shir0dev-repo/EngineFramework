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
struct Material;
struct PipelineSummary;
struct ShaderModule;
struct RenderCommand;

template<typename T>
class linkedList;
typedef linkedList<RenderCommand*> RenderCommandList;

struct GraphicsPipeline {
	const uint32_t MAX_MATERIAL_COUNT = 100;
	VkPipeline_T* const getPipeline();
	VkPipelineLayout_T* const getLayout() const;
	VkDescriptorSet_T* const getDescriptor(uint32_t copyIndex, uint32_t binding);
	uint32_t getDescriptorCopyCount() const;

	void getDescriptorsForEachFrame(uint32_t binding, uint32_t* count, VkDescriptorSet_T** outDescriptors);

	const PipelineMaterialLayout* const getMaterialLayout() const;
	const PipelineSummary* const getSummary() const;
	
	void setup(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment);
	void teardown(VkDevice_T* logicalDevice);

	void bindDescriptorSets(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame);

	void addRenderCommand(const MeshRenderer* const meshRenderer);
	void executeRenderCommands(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame);

	void generateMaterialDescriptorSets(const VulkanInstance* const instance, VkDescriptorSet_T** outSets);
	void generateInstanceDescriptorSets(const VulkanInstance* const instance, VkDescriptorSet_T** outSets);
	void registerMaterial(Material* material);
	void getRegisteredMaterials(uint32_t* count, Material** outMaterials);
private:
	static VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(const ShaderModule*& shader);
	void createPipelineSummary(const VulkanInstance* const instance, ShaderModule* vertex, ShaderModule* fragment);
	void createPipeline(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment);
	void createMaterialLayout(const VulkanInstance* const instance);

	void setupDescriptors(const VulkanInstance* const instance);
	void createPipelineDescriptorPool(const VulkanInstance* const instance);
	void createPipelineDescriptorSetLayout(const VulkanInstance* const instance, VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t layoutCount);
	void createPipelineDescriptorSets(const VulkanInstance* const instance);
	void updatePipelineDescriptorWrites(const VulkanInstance* const instance);

	void createMaterialDescriptorPool(const VulkanInstance* const instance);
	void createMaterialDescriptorSetLayout(const VulkanInstance* const instance, VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t layoutCount);
	
	void createInstanceDescriptorPool(const VulkanInstance* const instance);
	void createInstanceDescriptorSetLayout(const VulkanInstance* const instance, VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t layoutCount);

	void cleanupRenderCommands();
	
	PipelineSummary* summary = nullptr;
	VkPipeline_T* vkPipeline = nullptr;
	VkPipelineLayout_T* vkLayout = nullptr;

	VkDescriptorPool_T* vkPipelineDescriptorPool = nullptr;
	VkDescriptorSetLayout_T* vkPipelineDescriptorLayout = nullptr;
	VkDescriptorSet_T** vkPipelineDescriptorSets = nullptr;

	VkDescriptorPool_T* vkMaterialDescriptorPool = nullptr;
	VkDescriptorSetLayout_T* vkMaterialDescriptorLayout = nullptr;
	
	VkDescriptorPool_T* vkInstanceDescriptorPool = nullptr;
	VkDescriptorSetLayout_T* vkInstanceDescriptorLayout = nullptr;

	RenderCommandList* commandList = nullptr;

	PipelineMaterialLayout materialLayout;

	uint32_t numDescriptorCopies = 0;
	uint32_t numPipelineDescriptorSets = 0;
	uint32_t numMaterialDescriptorSets = 0;
	uint32_t numInstanceDescriptorSets = 0;
};
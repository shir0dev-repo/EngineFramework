#include "Core/Graphics/Pipeline/GraphicsPipeline.h"

#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"
#include "Core/Vulkan/VulkanSwapChain.h"

#include "Core/Structure/linkedList.h"

#include "Core/Graphics/Descriptor/Descriptors.h"
#include "Core/Graphics/Descriptor/DescriptorHandle.h"
#include "Core/Graphics/Material/Material.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Graphics/RenderCommand.h"
#include "Core/Graphics/Shader/ShaderModule.h"
#include "Core/Graphics/Shader/ShaderModuleInfo.h"
#include "Core/Graphics/Shader/Reflection/PipelineSummary.h"
#include "Core/Graphics/Shader/Reflection/DescriptorBindingInfo.h"
#include "Core/Graphics/Shader/Reflection/PushConstantInfo.h"
#include "Core/Graphics/Shader/Reflection/VertexAttributeInfo.h"
#include "Core/Graphics/Shader/Reflection/FragmentOutputInfo.h"
#include "Core/Graphics/Uniform/UniformBuffer.h"
#include "Core/Graphics/RenderPipelineInfo.h"
#include "Core/Graphics/Texture/GPUTexture.h"
#include "Core/Graphics/Mesh/Mesh.h"

#include "Core/Entity/Entity.h"
//#include "Core/Entity/Component/MeshRenderer.h"
#include "Core/Entity/Component/IRenderable.h"

#include <string>
#include <vulkan/vulkan.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>

static std::unordered_map<std::string, std::string> staticAssetFilePaths {
	std::make_pair("white", "Assets/Textures/white.jpg"),
	std::make_pair("noise", "Assets/Textures/noise.jpg"),
	std::make_pair("normal", "Assets/Textures/flat-normal.png")
};

static std::unordered_map <GraphicsPipeline*, std::vector<Material*>> registeredMaterialLookup;

static const char* getFilePathForTexture(const char* name) {
	std::string str(name);
	const char* cstr = str.c_str();
	auto it = staticAssetFilePaths.find(cstr);
	if (it != staticAssetFilePaths.end()) {
		return it->second.c_str();
	}
	else {
		return nullptr;
	}
}
using BindingList = std::vector<VkDescriptorSetLayoutBinding>;
using SetBindingMap = std::unordered_map<uint32_t, BindingList>;

using SetLayoutList = std::vector<VkDescriptorSetLayout>;
using PCRangeList = std::vector<VkPushConstantRange>;
using VertexAttributeList = std::vector<VkVertexInputAttributeDescription>;

#pragma region STATICS
	#pragma region Descriptor Set Layouts
static BindingList mergeDescriptorSetLayouts(DescriptorBindingInfo* bindingInfos, uint32_t infoCount) {
	BindingList bindingsPerSet{};

	for (uint32_t i = 0; i < infoCount; i++) {
		DescriptorBindingInfo b = bindingInfos[i];
		VkDescriptorSetLayoutBinding vk{};
		vk.binding = b.bindingIndex;
		vk.descriptorType = b.type;
		vk.descriptorCount = b.count;
		vk.stageFlags = b.stages;
		vk.pImmutableSamplers = nullptr;
		
		bindingsPerSet.push_back(vk);
	}

	return bindingsPerSet;
}

static void createDescriptorSetLayouts(VkDevice_T* logicalDevice, Renderer* renderer, const SetBindingMap& bindingsPerSet, SetLayoutList* setLayouts) {
	for (auto& [set, bindings] : bindingsPerSet) {
		if (set != Renderer::PIPELINE_DESCRIPTOR_SET) {
			continue;
		}

		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();
		
		VkDescriptorSetLayout layout;
		if (vkCreateDescriptorSetLayout(logicalDevice, &createInfo, nullptr, &layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout!");
		}

		setLayouts->push_back(layout);
	}
}
#pragma endregion

	#pragma region Merge Push Constants
static void mergePushConstantRanges(PushConstantInfo* pcInfos, uint32_t infoCount, std::vector<VkPushConstantRange>* pcRanges) {
	for (uint32_t i = 0; i < infoCount; i++) {
		PushConstantInfo info = pcInfos[i];
		VkPushConstantRange range = {};
		range.offset = info.offset;
		range.size = info.size;
		range.stageFlags = info.stages;
		pcRanges->push_back(range);
	}
}
	#pragma endregion

	#pragma region Merge Vertex Input Attributes
static void mergeVertexInputAttributes(VertexAttributeInfo* attribInfos, uint32_t infoCount, VertexAttributeList* attributes) {
	for (uint32_t i = 0; i < infoCount; i++) {
		VertexAttributeInfo info = attribInfos[i];

		VkVertexInputAttributeDescription desc = {};
		desc.location = info.location;
		desc.binding = 0;
		desc.format = info.format;
		desc.offset = info.offset;

		attributes->push_back(desc);
	}
}
	#pragma endregion

	#pragma region Create Pipeline Layout
static void createPipelineLayout(const VulkanContext* const instance, Renderer* renderer, VkDescriptorSetLayout_T* pipelineSetLayout,
	VkDescriptorSetLayout_T* materialSetLayout, VkDescriptorSetLayout_T* instanceSetLayout, const PCRangeList& pcRanges, VkPipelineLayout_T** outLayout) {
	
	std::vector<VkDescriptorSetLayout> sets{ renderer->getGlobalDescriptorSetLayout(), pipelineSetLayout, materialSetLayout, instanceSetLayout };

	VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = sets.size();
	createInfo.pSetLayouts = sets.data();
	createInfo.pushConstantRangeCount = pcRanges.size();
	createInfo.pPushConstantRanges = pcRanges.data();
	
	if (vkCreatePipelineLayout(instance->getDevice()->getLogicalDevice(), &createInfo, nullptr, outLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}
	#pragma endregion

	#pragma region Shader Stage Create Info
VkPipelineShaderStageCreateInfo GraphicsPipeline::makeShaderStageCreateInfo(const ShaderModule*& shader) {
	ShaderModuleInfo* info = nullptr;
	if (!ShaderModule::getInfo(*shader, &info)) {
		throw std::runtime_error("Failed to find shader info!");
	}

	VkPipelineShaderStageCreateInfo out = {};
	out.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	out.stage = info->stage;
	out.module = shader->getModule();
	out.pName = info->entry;

	return out;
}
	#pragma endregion
#pragma endregion

VkPipeline_T* const GraphicsPipeline::getPipeline() {
	return vkPipeline;
}

VkPipelineLayout_T* const GraphicsPipeline::getLayout() const {
	return vkLayout;
}

VkDescriptorSet_T* const GraphicsPipeline::getDescriptor(uint32_t copyIndex, uint32_t binding) {
	return vkPipelineDescriptorSets[copyIndex];
}

uint32_t GraphicsPipeline::getDescriptorCopyCount() const { return numDescriptorCopies; }

void GraphicsPipeline::getDescriptorsForEachFrame(uint32_t binding, uint32_t* count, VkDescriptorSet_T** outDescriptors) {
	*count = numDescriptorCopies;
	if (outDescriptors == nullptr) {
		return;
	}

	for (uint32_t i = 0; i < *count; i++) {
		outDescriptors[i] = getDescriptor(i, binding);
	}
}

const PipelineMaterialLayout* const GraphicsPipeline::getMaterialLayout() const {
	return &this->materialLayout;
}

const PipelineSummary* const GraphicsPipeline::getSummary() const {
	return this->summary;
}

void GraphicsPipeline::setup(const VulkanContext* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment,
	const bool transparency) {
	this->commandList = new linkedList<RenderCommand*>();

	createPipelineSummary(instance, vertex, fragment);
	createMaterialLayout(instance);
	
	setupDescriptors(instance);
	createPipeline(instance, renderer, vertex, fragment, transparency);
}

void GraphicsPipeline::createPipelineSummary(const VulkanContext* const instance, ShaderModule* vertex, ShaderModule* fragment) {
	ShaderModule* shaders[] = { vertex, fragment };
	this->summary = PipelineSummary::createSummary(shaders, 2);
}

void GraphicsPipeline::setupDescriptors(const VulkanContext* const instance) {
	this->numDescriptorCopies = instance->getSwapChain()->getSwapChainImageCount();
	this->numPipelineDescriptorSets = summary->pipelineDescriptors.count;
	this->numMaterialDescriptorSets = summary->materialDescriptors.count;
	this->numInstanceDescriptorSets = summary->instanceDescriptors.count;

	BindingList pipelineBindings = mergeDescriptorSetLayouts(summary->pipelineDescriptors.pDescriptorBindingInfos, summary->pipelineDescriptors.count);
	createPipelineDescriptorPool(instance);
	createPipelineDescriptorSetLayout(instance, pipelineBindings.data(), pipelineBindings.size());
	createPipelineDescriptorSets(instance);
	updatePipelineDescriptorWrites(instance);

	BindingList materialBindings = mergeDescriptorSetLayouts(summary->materialDescriptors.pDescriptorBindingInfos, summary->materialDescriptors.count);
	createMaterialDescriptorPool(instance);
	createMaterialDescriptorSetLayout(instance, materialBindings.data(), materialBindings.size());

	BindingList instanceBindings = mergeDescriptorSetLayouts(summary->instanceDescriptors.pDescriptorBindingInfos, summary->instanceDescriptors.count);
	createInstanceDescriptorPool(instance);
	createInstanceDescriptorSetLayout(instance, instanceBindings.data(), instanceBindings.size());
}

void GraphicsPipeline::createPipeline(const VulkanContext* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment,
	const bool transparency) {
	#pragma region Push Constants
	std::vector<VkPushConstantRange> pcRanges;
	mergePushConstantRanges(summary->pPushConstantInfos, summary->numPushConstantInfos, &pcRanges);
	#pragma endregion

	createPipelineLayout(instance, renderer, 
		this->vkPipelineDescriptorLayout, this->vkMaterialDescriptorLayout, this->vkInstanceDescriptorLayout,
		pcRanges, &this->vkLayout);

	#pragma region ShaderInfo
	VertexAttributeList vertexAttributes;
	mergeVertexInputAttributes(summary->pVertexInputAttributeInfos, summary->numVertexInputAttributeInfos, &vertexAttributes);
	VkVertexInputBindingDescription vertexBindingDesc = {};
	vertexBindingDesc.binding = 0;
	vertexBindingDesc.stride = sizeof(Vertex);
	vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertexCreateInfo = {};
	vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexCreateInfo.vertexBindingDescriptionCount = 1;
	vertexCreateInfo.pVertexBindingDescriptions = &vertexBindingDesc;
	vertexCreateInfo.vertexAttributeDescriptionCount = vertexAttributes.size();
	vertexCreateInfo.pVertexAttributeDescriptions = vertexAttributes.data();

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vertex->getModule();
	stages[0].pName = "main";

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fragment->getModule();
	stages[1].pName = "main";
	#pragma endregion

	// TODO: Add specialization constants

	#pragma region Assembly
	VkPipelineInputAssemblyStateCreateInfo assembly = {};
	assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	#pragma endregion

	#pragma region Viewport
	VkPipelineViewportStateCreateInfo viewport = {};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport.viewportCount = 1;
	viewport.scissorCount = 1;
	#pragma endregion

	#pragma region Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;

	#pragma endregion

	#pragma region Dynamic States
	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();
	#pragma endregion

	#pragma region Multisampling
	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	#pragma endregion

	#pragma region Blending
	VkPipelineColorBlendAttachmentState blend = {};
	blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (transparency) {
		blend.blendEnable = VK_TRUE; // transparency 
		blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blend.colorBlendOp = VK_BLEND_OP_ADD;
		blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	else {
		blend.blendEnable = VK_FALSE;
	}

	VkPipelineColorBlendStateCreateInfo blendState = {};
	blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendState.attachmentCount = 1;
	blendState.pAttachments = &blend;
	#pragma endregion
	
	#pragma region Depth Stencil
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = transparency ? VK_FALSE : VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS;
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.minDepthBounds = 0.0f;
	ds.maxDepthBounds = 1.0f;
	ds.stencilTestEnable = VK_FALSE;
	ds.front = {};
	ds.back = {};
	#pragma endregion

	#pragma region Pipeline Creation
	VkGraphicsPipelineCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = &(*stages);
	createInfo.pDynamicState = &dynamicState;
	createInfo.pVertexInputState = &vertexCreateInfo;
	createInfo.pInputAssemblyState = &assembly;
	createInfo.pViewportState = &viewport;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &ms;
	createInfo.pDepthStencilState = &ds;
	createInfo.pColorBlendState = &blendState;
	createInfo.layout = this->vkLayout;
	createInfo.renderPass = renderer->getRenderPass();
	createInfo.subpass = 0;
	if (vkCreateGraphicsPipelines(instance->getDevice()->getLogicalDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &this->vkPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
	#pragma endregion
}

void GraphicsPipeline::createMaterialLayout(const VulkanContext* const instance) {
	if (summary->materialDescriptors.count + summary->instanceDescriptors.count <= 0) {
		this->materialLayout.numBindings = 0;
		this->materialLayout.pBindings = nullptr;
	}

	this->materialLayout.numBindings = summary->materialDescriptors.count + summary->instanceDescriptors.count;
	this->materialLayout.pBindings = new MaterialBinding[materialLayout.numBindings];
	
	for (uint32_t i = 0; i < summary->materialDescriptors.count; i++) {
		DescriptorBindingInfo info = summary->materialDescriptors.pDescriptorBindingInfos[i];
		MaterialBinding* binding = &this->materialLayout.pBindings[i];

		binding->name = info.name;
		binding->type = info.type;
		binding->binding = info.bindingIndex;
		binding->set = info.setIndex;
	}

	for (uint32_t j = summary->materialDescriptors.count, i = 0; i < summary->instanceDescriptors.count; i++) {
		DescriptorBindingInfo info = summary->instanceDescriptors.pDescriptorBindingInfos[i];
		MaterialBinding* binding = &this->materialLayout.pBindings[j++];

		binding->name = info.name;
		binding->type = info.type;
		binding->binding = info.bindingIndex;
		binding->set = info.setIndex;
	}
}

void GraphicsPipeline::createPipelineDescriptorPool(const VulkanContext* const instance) {
	VkDescriptorPoolSize poolSizes[2] = { {}, {} };
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = numPipelineDescriptorSets * numDescriptorCopies;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = numPipelineDescriptorSets * numDescriptorCopies;
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = &poolSizes[0];
	poolInfo.maxSets = numDescriptorCopies * numPipelineDescriptorSets * 2;

	if (poolInfo.maxSets <= 0) {
		return;
	}

	if (vkCreateDescriptorPool(instance->getDevice()->getLogicalDevice(), &poolInfo, nullptr, &this->vkPipelineDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline descriptor pool!");
	}
}

void GraphicsPipeline::createMaterialDescriptorPool(const VulkanContext* const instance) {
	VkDescriptorPoolSize poolSizes[2] = { {}, {} };
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = numDescriptorCopies * MAX_MATERIAL_COUNT;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = numDescriptorCopies * MAX_MATERIAL_COUNT;
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = &poolSizes[0];
	poolInfo.maxSets = numDescriptorCopies * 2 * MAX_MATERIAL_COUNT;

	if (poolInfo.maxSets <= 0) {
		return;
	}

	if (vkCreateDescriptorPool(instance->getDevice()->getLogicalDevice(), &poolInfo, nullptr, &this->vkMaterialDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create material descriptor pool!");
	}
}
void GraphicsPipeline::createInstanceDescriptorPool(const VulkanContext* const instance) {
	VkDescriptorPoolSize poolSizes[1] = { {} };
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = numDescriptorCopies * MAX_MATERIAL_COUNT;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSizes[0];
	poolInfo.maxSets = numDescriptorCopies * 2 * MAX_MATERIAL_COUNT;

	if (vkCreateDescriptorPool(instance->getDevice()->getLogicalDevice(), &poolInfo, nullptr, &this->vkInstanceDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance descriptor pool!");
	}
}

void GraphicsPipeline::createPipelineDescriptorSetLayout(const VulkanContext* const instance, VkDescriptorSetLayoutBinding* layoutBindings, uint32_t count) {
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = count;
	createInfo.pBindings = layoutBindings;

	if (vkCreateDescriptorSetLayout(instance->getDevice()->getLogicalDevice(), &createInfo, nullptr, &this->vkPipelineDescriptorLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline descriptor set layout!");
	}
}

void GraphicsPipeline::createMaterialDescriptorSetLayout(const VulkanContext* const instance, VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t layoutCount) {
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = layoutCount;
	createInfo.pBindings = setLayoutBindings;
	
	if (vkCreateDescriptorSetLayout(instance->getDevice()->getLogicalDevice(), &createInfo, nullptr, &this->vkMaterialDescriptorLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create material descriptor set layout!");
	}
}


void GraphicsPipeline::createInstanceDescriptorSetLayout(const VulkanContext* const instance, VkDescriptorSetLayoutBinding* setLayoutBindings, uint32_t layoutCount) {
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = layoutCount;
	createInfo.pBindings = setLayoutBindings;

	if (vkCreateDescriptorSetLayout(instance->getDevice()->getLogicalDevice(), &createInfo, nullptr, &this->vkInstanceDescriptorLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create material descriptor set layout!");
	}
}

void GraphicsPipeline::createPipelineDescriptorSets(const VulkanContext* const instance) {
	if (numPipelineDescriptorSets <= 0) {
		return;
	}

	// one for each frame
	this->vkPipelineDescriptorSets = new VkDescriptorSet_T* [numDescriptorCopies] { nullptr };
	for (uint32_t i = 0; i < numDescriptorCopies; i++) {
		std::vector<VkDescriptorSetLayout_T*> layouts(this->numPipelineDescriptorSets, this->vkPipelineDescriptorLayout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = this->vkPipelineDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts.data();
		
		VkResult allocResult = vkAllocateDescriptorSets(instance->getDevice()->getLogicalDevice(), &allocInfo, &this->vkPipelineDescriptorSets[i]);
		if (allocResult != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}
	}
}

void GraphicsPipeline::generateMaterialDescriptorSets(const VulkanContext* const instance, VkDescriptorSet_T** outSets) {
	std::vector<VkDescriptorSetLayout_T*> layouts(this->numDescriptorCopies, this->vkMaterialDescriptorLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = vkMaterialDescriptorPool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(instance->getDevice()->getLogicalDevice(), &allocInfo, outSets) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate material descriptor sets!");
	}
}

void GraphicsPipeline::generateInstanceDescriptorSets(const VulkanContext* const instance, VkDescriptorSet_T** outSets) {
	std::vector<VkDescriptorSetLayout_T*> layouts(this->numDescriptorCopies, this->vkInstanceDescriptorLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = vkInstanceDescriptorPool;
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(instance->getDevice()->getLogicalDevice(), &allocInfo, outSets) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate material descriptor sets!");
	}
}

void GraphicsPipeline::updatePipelineDescriptorWrites(const VulkanContext* const instance) {
	uint32_t numDescriptors = summary->pipelineDescriptors.count;
	std::vector<VkDescriptorImageInfo> imageInfos = {};
	std::vector<VkDescriptorBufferInfo> bufferInfos = {};
	
	for (uint32_t copyIndex = 0; copyIndex < numDescriptorCopies; copyIndex++) {
		std::vector<VkWriteDescriptorSet> descriptorWrites = {};
		
		for (uint32_t i = 0; i < numDescriptors; i++) {
			DescriptorBindingInfo b = summary->pipelineDescriptors.pDescriptorBindingInfos[i];

			if (b.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				const char* filePath = staticAssetFilePaths[std::string(b.name)].c_str();
				GPUTexture* texture = GPUTexture::createTextureLoadImmediate(instance, filePath, b.name);
				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = texture->imageView;
				imageInfo.sampler = texture->imageSampler;
				imageInfos.push_back(imageInfo);

				VkWriteDescriptorSet write = Descriptors::makeImageSamplerDescriptorWrite(vkPipelineDescriptorSets[copyIndex], b.bindingIndex, &imageInfo);
				vkUpdateDescriptorSets(instance->getDevice()->getLogicalDevice(), 1, &write, 0, nullptr);
			}
		}
	}
}

void GraphicsPipeline::bindDescriptorSets(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) {
	if (vkPipelineDescriptorSets != nullptr) {
		VkDescriptorSet_T* boundSets = vkPipelineDescriptorSets[currentFrame];
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkLayout, Renderer::PIPELINE_DESCRIPTOR_SET, 1, &boundSets, 0, nullptr);
	}
}

void GraphicsPipeline::registerMaterial(Material* material) {
	auto it = registeredMaterialLookup.find(this);
	if (it == registeredMaterialLookup.end()) {
		registeredMaterialLookup[this] = {};
	}
	std::vector<Material*>* materials = &registeredMaterialLookup[this];
	for (auto* registered : *materials) {
		if (registered == material) {
			return;
		}
	}

	materials->push_back(material);
}

void GraphicsPipeline::getRegisteredMaterials(uint32_t* count, Material** outMaterials) {
	*count = 0;
	auto it = registeredMaterialLookup.find(this);
	if (it != registeredMaterialLookup.end()) {
		*count = it->second.size();
	}

	if (*count == 0 || outMaterials == nullptr) {
		return;
	}

	for (uint32_t i = 0; i < it->second.size(); i++) {
		outMaterials[i] = it->second.at(i);
	}
}

void GraphicsPipeline::addRenderCommand(const IRenderable* const meshRenderer) {
	RenderCommand* rc = RenderCommand::create(
		meshRenderer->getMaterial(),
		meshRenderer->getVertexBuffer(), meshRenderer->getIndexBuffer(),
		meshRenderer->getVertexCount(), meshRenderer->getIndexCount(),
		meshRenderer->getTransformBuffer());

	commandList->add(rc);
}

void GraphicsPipeline::executeRenderCommands(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) {
	Material* currentMaterial = nullptr;

	for (uint32_t i = 0; i < commandList->size(); i++) {
		if (currentMaterial != (*commandList)[i]->material) {
			currentMaterial = (*commandList)[i]->material;
			currentMaterial->bind(commandBuffer, currentFrame);
		}

		(*commandList)[i]->execute(commandBuffer);
	}

	cleanupRenderCommands();
}

void GraphicsPipeline::teardown(VkDevice_T* logicalDevice) {
	cleanupRenderCommands();
	delete commandList;
	if (vkPipeline != nullptr) {
		vkDestroyPipeline(logicalDevice, vkPipeline, nullptr);
		vkPipeline = nullptr;
	}

	if (vkPipelineDescriptorLayout != nullptr) {
		vkDestroyDescriptorSetLayout(logicalDevice, vkPipelineDescriptorLayout, nullptr);
		vkPipelineDescriptorLayout = nullptr;
		numPipelineDescriptorSets = 0;
	}
	if (vkMaterialDescriptorLayout != nullptr) {
		vkDestroyDescriptorSetLayout(logicalDevice, vkMaterialDescriptorLayout, nullptr);
		vkMaterialDescriptorLayout = nullptr;
	}
	if (vkInstanceDescriptorLayout != nullptr) {
		vkDestroyDescriptorSetLayout(logicalDevice, vkInstanceDescriptorLayout, nullptr);
		vkInstanceDescriptorLayout = nullptr;
	}

	if (vkPipelineDescriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDevice, vkPipelineDescriptorPool, nullptr);
		vkPipelineDescriptorPool = nullptr;
	}
	if (vkMaterialDescriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDevice, vkMaterialDescriptorPool, nullptr);
		vkMaterialDescriptorPool = nullptr;
	}
	if (vkInstanceDescriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDevice, vkInstanceDescriptorPool, nullptr);
		vkInstanceDescriptorPool = nullptr;
	}

	if (vkLayout != nullptr) {
		vkDestroyPipelineLayout(logicalDevice, vkLayout, nullptr);
		vkLayout = nullptr;
	}

	numPipelineDescriptorSets = 0;
}

void GraphicsPipeline::cleanupRenderCommands() {
	for (uint32_t i = 0; i < commandList->size(); i++) {
		RenderCommand* rc = (*commandList)[i];
		if (rc) {
			delete rc;
		}
	}

	commandList->clear();
}
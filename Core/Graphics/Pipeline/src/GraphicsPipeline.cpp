#include "../GraphicsPipeline.h"
#include "../../Renderer/Renderer.h"
#include "../../RenderCommand.h"
#include "../../Shader/ShaderModule.h"
#include "../../Shader/ShaderModuleInfo.h"
#include "../../Shader/Reflection/PipelineSummary.h"
#include "../../Shader/Reflection/DescriptorBindingInfo.h"
#include "../../Shader/Reflection/PushConstantInfo.h"
#include "../../Shader/Reflection/VertexAttributeInfo.h"
#include "../../Shader/Reflection/FragmentOutputInfo.h"
#include "../../Shader/Vertex.h"
#include "../../Uniform/UniformBuffer.h"
#include "../../RenderPipelineInfo.h"
#include "../../../Vulkan/VulkanInstance.h"
#include "../../../Vulkan/VulkanDevice.h"
#include "../../../Vulkan/VulkanSwapChain.h"
#include "../../../Structure/linkedList.h"

#include "../../Renderer/MeshRenderer.h"
#include "../../Mesh/Mesh.h"

#include <vulkan/vulkan.h>
#include <iostream>
#include <unordered_map>
#include <vector>

using BindingList = std::vector<VkDescriptorSetLayoutBinding>;
using SetBindingMap = std::unordered_map<uint32_t, BindingList>;
using SetLayoutList = std::vector<VkDescriptorSetLayout>;
using PCRangeList = std::vector<VkPushConstantRange>;
using VertexAttributeList = std::vector<VkVertexInputAttributeDescription>;

#pragma region STATICS
	#pragma region Descriptor Set Layouts
static void mergeDescriptorSetLayouts(DescriptorBindingInfo* bindingInfos, uint32_t infoCount, SetBindingMap* bindingsPerSet) {
	for (uint32_t i = 0; i < infoCount; i++) {
		DescriptorBindingInfo b = bindingInfos[i];
		VkDescriptorSetLayoutBinding vk{};
		vk.binding = b.bindingIndex;
		vk.descriptorType = b.type;
		vk.descriptorCount = b.count;
		vk.stageFlags = b.stages;
		vk.pImmutableSamplers = nullptr;

		(*bindingsPerSet)[b.setIndex].push_back(vk);
	}
}

static void createDescriptorSetLayouts(VkDevice_T* logicalDevice, Renderer* renderer, const SetBindingMap& bindingsPerSet, SetLayoutList* setLayouts) {
	const int globalSet = 0, globalBinding = 0;
	for (auto& [set, bindings] : bindingsPerSet) {
		if (set == globalSet) {
			bool isGlobalBinding = false;
			for (auto& b : bindings) {
				if (b.binding == globalBinding) {
					isGlobalBinding = true;
					break;
				}
			}
			if (isGlobalBinding) {
				continue;
			}
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
static void createPipelineLayout(const VulkanDevice* const device, Renderer* renderer, SetLayoutList* setLayout, const PCRangeList& pcRanges,
	VkPipelineLayout_T** outLayout) {
	
	
	setLayout->push_back(renderer->getGlobalDescriptorSetLayout());

	VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = setLayout->size();
	createInfo.pSetLayouts = setLayout->data();
	createInfo.pushConstantRangeCount = pcRanges.size();
	createInfo.pPushConstantRanges = pcRanges.data();
	
	if (vkCreatePipelineLayout(device->logicalDevice, &createInfo, nullptr, outLayout) != VK_SUCCESS) {
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

void GraphicsPipeline::setup(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment) {
	this->commandList = new linkedList<RenderCommand*>();
	this->numDescriptorSets = instance->swapChain->getSwapChainImageCount();

	createPipeline(instance, renderer, vertex, fragment);
	createDescriptorPool(instance);
	createDescriptorSets(instance, vkDescriptorSetLayouts, numSetLayouts);
}

void GraphicsPipeline::createPipeline(const VulkanInstance* const instance, Renderer* renderer, ShaderModule* vertex, ShaderModule* fragment) {
	ShaderModule* shaders[] = { vertex, fragment };
	this->summary = PipelineSummary::createSummary(shaders, 2);

	#pragma region Descriptor Set Layouts
	SetBindingMap bindingsPerSet;
	mergeDescriptorSetLayouts(summary->pDescriptorBindingInfos, summary->numDescriptorBindingInfos, &bindingsPerSet);

	std::vector<VkDescriptorSetLayout> setLayouts;
	createDescriptorSetLayouts(instance->device->logicalDevice, renderer, bindingsPerSet, &setLayouts);
	this->numSetLayouts = setLayouts.size();
	if (numSetLayouts > 0) {
		this->vkDescriptorSetLayouts = new VkDescriptorSetLayout_T* [numSetLayouts];
		memcpy(this->vkDescriptorSetLayouts, setLayouts.data(), sizeof(VkDescriptorSetLayout) * numSetLayouts);
	}
	#pragma endregion

	#pragma region Push Constants
	std::vector<VkPushConstantRange> pcRanges;
	mergePushConstantRanges(summary->pPushConstantInfos, summary->numPushConstantInfos, &pcRanges);
	#pragma endregion

	createPipelineLayout(instance->device, renderer, &setLayouts, pcRanges, &this->vkLayout);

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
	VkPipelineColorBlendAttachmentState blend = {};
	blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blend.blendEnable = VK_FALSE; // transparency 
	VkPipelineColorBlendStateCreateInfo blendState = {};
	blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendState.attachmentCount = 1;
	blendState.pAttachments = &blend;
	#pragma endregion
	
	#pragma region Depth Stencil
	VkPipelineDepthStencilStateCreateInfo ds = {};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS;
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
	if (vkCreateGraphicsPipelines(instance->device->logicalDevice, VK_NULL_HANDLE, 1, &createInfo, nullptr, &this->vkPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
	#pragma endregion
}

void GraphicsPipeline::createDescriptorPool(const VulkanInstance* const instance) {
	uint32_t numFramebuffers = instance->swapChain->getSwapChainImageCount();
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = numFramebuffers;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = numFramebuffers;

	if (vkCreateDescriptorPool(instance->device->logicalDevice, &poolInfo, nullptr, &this->vkDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void GraphicsPipeline::createDescriptorSets(const VulkanInstance* const instance, VkDescriptorSetLayout_T** setLayouts, uint32_t layoutCount) {
	if (numSetLayouts <= 0) {
		return;
	}

	this->vkDescriptorSets = new VkDescriptorSet_T** [layoutCount] { nullptr };

	for (uint32_t i = 0; i < layoutCount; i++) {
		std::vector<VkDescriptorSetLayout_T*> layouts(this->numDescriptorSets, setLayouts[i]);
		
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = vkDescriptorPool;
		allocInfo.descriptorSetCount = this->numDescriptorSets;
		allocInfo.pSetLayouts = layouts.data();
		this->vkDescriptorSets[i] = new VkDescriptorSet_T* [this->numDescriptorSets] { nullptr };
		VkResult allocResult = vkAllocateDescriptorSets(instance->device->logicalDevice, &allocInfo, this->vkDescriptorSets[i]);
		if (allocResult != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}
	}
}

void GraphicsPipeline::createUniformBuffers(const VulkanInstance* const instance) {

}

void GraphicsPipeline::updateDescriptorSets(const VulkanInstance* const instance) {
	for (uint32_t i = 0; i < numDescriptorSets; i++) {
		VkDescriptorBufferInfo bufferInfo = {};

	}
}

void GraphicsPipeline::addRenderCommand(const MeshRenderer* const meshRenderer) {
	RenderCommand* rc = RenderCommand::create(
		meshRenderer->getVertexBuffer(), meshRenderer->getIndexBuffer(),
		meshRenderer->mesh->vertexCount, meshRenderer->mesh->indexCount);

	commandList->add(rc);
}

void GraphicsPipeline::executeRenderCommands(VkCommandBuffer_T* commandBuffer) {
	for (uint32_t i = 0; i < commandList->size(); i++) {
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
	if (vkDescriptorSetLayouts != nullptr) {
		for (uint32_t i = 0; i < numSetLayouts; i++) {
			vkDestroyDescriptorSetLayout(logicalDevice, vkDescriptorSetLayouts[i], nullptr);
		}
		delete[] vkDescriptorSetLayouts;
		vkDescriptorSetLayouts = nullptr;
		numSetLayouts = 0;
	}
	if (vkDescriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDevice, vkDescriptorPool, nullptr);
		vkDescriptorPool = nullptr;
	}
	if (vkLayout != nullptr) {
		vkDestroyPipelineLayout(logicalDevice, vkLayout, nullptr);
		vkLayout = nullptr;
	}
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
#include "../RenderPipelineInfo.h"
#include "../Shader/Vertex.h"

#include <vulkan/vulkan.h>
#include<vector>
#include <iostream>

const std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
};

void RenderPipelineInfo::makePipelineVertexShaderStateCreateInfo(VkShaderModule_T* shader, VkPipelineShaderStageCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	createInfo.module = shader;
	createInfo.pName = "main";
}

void RenderPipelineInfo::makePipelineFragmentShaderStateCreateInfo(VkShaderModule_T* shader, VkPipelineShaderStageCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	createInfo.module = shader;
	createInfo.pName = "main";
}

void RenderPipelineInfo::makePipelineDynamicStateCreateInfo(VkPipelineDynamicStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	createInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	createInfo.pDynamicStates = dynamicStates.data();
}

void RenderPipelineInfo::makePipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VkVertexInputBindingDescription* bindingDesc = nullptr;
	Vertex::getBindingDescription(bindingDesc);
	createInfo.vertexBindingDescriptionCount = 1;
	createInfo.pVertexBindingDescriptions = bindingDesc;

	uint32_t attribCount = 0;
	VkVertexInputAttributeDescription* attribDescs = nullptr;
	Vertex::getAttributeDescriptions(&attribCount, attribDescs);
	createInfo.vertexAttributeDescriptionCount = attribCount;
	createInfo.pVertexAttributeDescriptions = attribDescs;
}

void RenderPipelineInfo::makePipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.primitiveRestartEnable = VK_FALSE;
}

void RenderPipelineInfo::makePipelineViewportStateCreateInfo(const VkViewport* viewport, const VkRect2D* scissor, VkPipelineViewportStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	createInfo.viewportCount = 1;
	createInfo.pViewports = viewport;
	createInfo.scissorCount = 1;
	createInfo.pScissors = scissor;
}

void RenderPipelineInfo::makePipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	createInfo.depthClampEnable = VK_FALSE;
	createInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.polygonMode = VK_POLYGON_MODE_FILL;
	createInfo.lineWidth = 1.0f;
	createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	createInfo.depthBiasEnable = VK_FALSE;
}

void RenderPipelineInfo::makePipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	createInfo.sampleShadingEnable = VK_FALSE;
	createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void RenderPipelineInfo::makePipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& attachmentState) {
	attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentState.blendEnable = VK_FALSE;
}

void RenderPipelineInfo::makePipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* attachmentStates, uint32_t attachmentCount, VkPipelineColorBlendStateCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	createInfo.logicOpEnable = VK_FALSE;
	createInfo.logicOp = VK_LOGIC_OP_COPY;
	createInfo.attachmentCount = attachmentCount;
	createInfo.pAttachments = attachmentStates;
}

void RenderPipelineInfo::createPipelineLayout(VkDevice_T* logicalDevice, VkDescriptorSetLayout_T** descriptorSetLayout,
	VkPipelineLayout_T** pipelineLayout) {
	
	VkPipelineLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = 1;
	createInfo.pSetLayouts = descriptorSetLayout;
	if (vkCreatePipelineLayout(logicalDevice, &createInfo, nullptr, pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

GraphicsPipelineCreateParams RenderPipelineInfo::makeGraphicsPipelineCreateParams(VkPipelineShaderStageCreateInfo* shaderStages,
	VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
	VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportState,
	VkPipelineRasterizationStateCreateInfo* rasterizer, VkPipelineMultisampleStateCreateInfo* multisampler,
	VkPipelineColorBlendStateCreateInfo* blendState, VkPipelineLayout_T* pipelineLayout, VkRenderPass_T* renderPass) {

	GraphicsPipelineCreateParams params = {};
	params.shaderStages = shaderStages;
	params.dynamicState = dynamicState;
	params.vertexInputInfo = vertexInputInfo;
	params.inputAssembly = inputAssembly;
	params.viewportState = viewportState;
	params.rasterizer = rasterizer;
	params.multisampler = multisampler;
	params.blendState = blendState;
	params.pipelineLayout = pipelineLayout;
	params.renderPass = renderPass;
	return params;
}

void RenderPipelineInfo::createGraphicsPipelines(VkDevice_T* logicalDevice, const GraphicsPipelineCreateParams& createParams, VkPipeline_T*& pipeline) {
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = createParams.shaderStages;
	pipelineInfo.pVertexInputState = createParams.vertexInputInfo;
	pipelineInfo.pInputAssemblyState = createParams.inputAssembly;
	pipelineInfo.pViewportState = createParams.viewportState;
	pipelineInfo.pRasterizationState = createParams.rasterizer;
	pipelineInfo.pMultisampleState = createParams.multisampler;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = createParams.blendState;
	pipelineInfo.pDynamicState = createParams.dynamicState;
	pipelineInfo.layout = createParams.pipelineLayout;
	pipelineInfo.renderPass = createParams.renderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
}
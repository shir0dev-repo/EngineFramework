#pragma once

typedef unsigned int uint32_t;

struct VkShaderModule_T;
struct VkPipelineShaderStageCreateInfo;

struct VkPipelineDynamicStateCreateInfo;
typedef enum VkDynamicState;

struct VkPipelineVertexInputStateCreateInfo;

struct VkPipelineInputAssemblyStateCreateInfo;

struct VkViewport;
struct VkRect2D;
struct VkPipelineViewportStateCreateInfo;

struct VkPipelineRasterizationStateCreateInfo;
struct VkPipelineMultisampleStateCreateInfo;

struct VkPipelineColorBlendAttachmentState;
struct VkPipelineColorBlendStateCreateInfo;

struct VkDevice_T;
struct VkPipelineLayout_T;
struct VkRenderPass_T;

struct VkPipeline_T;

struct GraphicsPipelineCreateParams {
	VkPipelineShaderStageCreateInfo* shaderStages = nullptr;
	VkPipelineDynamicStateCreateInfo* dynamicState = nullptr;
	VkPipelineVertexInputStateCreateInfo* vertexInputInfo = nullptr;
	VkPipelineInputAssemblyStateCreateInfo* inputAssembly = nullptr;
	VkPipelineViewportStateCreateInfo* viewportState = nullptr;
	VkPipelineRasterizationStateCreateInfo* rasterizer = nullptr;
	VkPipelineMultisampleStateCreateInfo* multisampler = nullptr;
	VkPipelineColorBlendStateCreateInfo* blendState = nullptr;
	VkPipelineLayout_T* pipelineLayout = nullptr;
	VkRenderPass_T* renderPass = nullptr;
};

struct RenderPipelineInfo {
	static void makePipelineVertexShaderStateCreateInfo(VkShaderModule_T* shader, VkPipelineShaderStageCreateInfo& createInfo);
	static void makePipelineFragmentShaderStateCreateInfo(VkShaderModule_T* shader, VkPipelineShaderStageCreateInfo& createInfo);
	static void makePipelineDynamicStateCreateInfo(VkPipelineDynamicStateCreateInfo& createInfo);
	static void makePipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo);
	static void makePipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& createInfo);
	static void makePipelineViewportStateCreateInfo(const VkViewport* viewport, const VkRect2D* scissor, VkPipelineViewportStateCreateInfo& createInfo);
	static void makePipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& createInfo);
	static void makePipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo& createInfo);
	static void makePipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& attachmentState);
	static void makePipelineColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState* attachmentStates, uint32_t attachmentCount, VkPipelineColorBlendStateCreateInfo& createInfo);
	static GraphicsPipelineCreateParams makeGraphicsPipelineCreateParams(VkPipelineShaderStageCreateInfo* shaderStages,
		VkPipelineDynamicStateCreateInfo* dynamicState, VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
		VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportState,
		VkPipelineRasterizationStateCreateInfo* rasterizer, VkPipelineMultisampleStateCreateInfo* multisampler,
		VkPipelineColorBlendStateCreateInfo* blendState, VkPipelineLayout_T* pipelineLayout, VkRenderPass_T* renderPass);

	static void createPipelineLayout(VkDevice_T* logicalDevice, VkPipelineLayout_T** pipelineLayout);
	static void createGraphicsPipelines(VkDevice_T* logicalDevice, const GraphicsPipelineCreateParams& createInfo, VkPipeline_T** pipeline);
};
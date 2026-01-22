#include "../GraphicsPipeline.h"
#include "../../Vulkan/VulkanSwapChain.h"

#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>

static VkShaderModule createShaderModule(VkDevice_T* logicalDevice, const char* byteCode, const uint32_t& size) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode);
	
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}

void GraphicsPipeline::setup(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain) {
	char* vertexContent = nullptr;
	char* fragmentContent = nullptr;

	uint32_t vertexSize, fragmentSize;

	readFile("Assets/Shaders/vert.spv", vertexContent, vertexSize);
	readFile("Assets/Shaders/frag.spv", fragmentContent, fragmentSize);

	VkShaderModule vertexShaderModule = createShaderModule(logicalDevice, vertexContent, vertexSize);
	VkShaderModule fragmentShaderModule = createShaderModule(logicalDevice, fragmentContent, fragmentSize);

	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderCreateInfo.module = vertexShaderModule;
	vertexShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderCreateInfo.module = fragmentShaderModule;
	fragmentShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	const VkExtent2D* const extent = swapChain->GetExtents();
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent->width;
	viewport.height = (float)extent->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = *extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	this->pipelineLayout = new VkPipelineLayout_T*();
	if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, this->pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
}

void GraphicsPipeline::teardown(VkDevice_T* logicalDevice) {
	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(logicalDevice, *pipelineLayout, nullptr);
		delete pipelineLayout;
		pipelineLayout = nullptr;
	}
}

bool GraphicsPipeline::readFile(const char* filePath, char*& outFileContents, uint32_t& fileSize) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	fileSize = (size_t)file.tellg();
	if (outFileContents != nullptr) {
		delete[] outFileContents;
	}
	
	outFileContents = new char[fileSize];

	file.seekg(0);
	file.read(outFileContents, fileSize);
	file.close();

	return true;
}
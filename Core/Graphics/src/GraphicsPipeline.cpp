#include "../GraphicsPipeline.h"
#include "../../Vulkan/VulkanSwapChain.h"
#include "../../Vulkan/VulkanDevice.h"
#include "../../Vulkan/VulkanCommandPool.h"
#include "../GraphicsSyncObject.h"

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

void GraphicsPipeline::setup(const VulkanDevice* const device, const VulkanSwapChain* const swapChain, VkSurfaceKHR_T* surface) {
	setupRenderPass(device->logicalDevice, swapChain);
	setupPipelineLayout(device->logicalDevice, swapChain);
	setupFramebuffers(device->logicalDevice, swapChain);
	setupCommandPool(device, surface);
	setupSyncObject(device->logicalDevice);
}

void GraphicsPipeline::setupRenderPass(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain) {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChain->getFormat()->format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAttachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(logicalDevice, &createInfo, nullptr, &this->renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void GraphicsPipeline::setupPipelineLayout(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain) {
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

	const VkExtent2D* const extent = swapChain->getExtents();
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
	
	if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = this->pipelineLayout;
	pipelineInfo.renderPass = this->renderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
}

void GraphicsPipeline::setupFramebuffers(VkDevice_T* logicalDevice, const VulkanSwapChain* const swapChain) {
	uint32_t bufferCount = swapChain->getSwapChainImageCount();
	this->framebufferCount = bufferCount;
	framebuffers = new VkFramebuffer_T*[bufferCount];
	const VkExtent2D* const extent = swapChain->getExtents();

	for (uint32_t i = 0; i < bufferCount; i++) {
		VkImageView attachments[] = {
			swapChain->getImageView(i)
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = extent->width;
		framebufferInfo.height = extent->height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void GraphicsPipeline::setupCommandPool(const VulkanDevice* const device, VkSurfaceKHR_T* surface) {
	this->commandPool = new VulkanCommandPool();
	commandPool->setup(device, surface);
}

void GraphicsPipeline::setupSyncObject(VkDevice_T* logicalDevice) {
	this->syncObject = new GraphicsSyncObject();
	this->syncObject->setup(logicalDevice);
}

void GraphicsPipeline::beginRenderPass(const VulkanSwapChain* const swapChain) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = this->renderPass;
	renderPassInfo.framebuffer = framebuffers[currentFramebufferIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = *swapChain->getExtents();

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	VkCommandBuffer_T* const commandBuffer = commandPool->getCommandBuffer();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
	initViewportScissor(commandBuffer, swapChain->getExtents());
}

void GraphicsPipeline::addRenderCommmand() {
	vkCmdDraw(commandPool->getCommandBuffer(), 3, 1, 0, 0);
}

void GraphicsPipeline::finalizeRenderPass(const VulkanSwapChain* const swapChain) {
	VkCommandBuffer_T* cmdBuffer = commandPool->getCommandBuffer();
	vkCmdEndRenderPass(cmdBuffer);

	if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void GraphicsPipeline::render(const VulkanDevice* const device, const VulkanSwapChain* const swapChain) {
	syncObject->wait(device->logicalDevice);
	syncObject->reset(device->logicalDevice);

	vkAcquireNextImageKHR(device->logicalDevice, swapChain->getSwapChain(), UINT64_MAX, syncObject->imageAvailableSemaphore, VK_NULL_HANDLE, &currentFramebufferIndex);
	
	commandPool->resetCommandBuffer();
	commandPool->beginRecordingCommandBuffer();

	beginRenderPass(swapChain);
	addRenderCommmand();
	finalizeRenderPass(swapChain);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore waitSemaphores[] = { syncObject->imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	VkCommandBuffer_T* commandBuffer = commandPool->getCommandBuffer();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VkSemaphore signalSemaphores[] = { syncObject->renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, syncObject->inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &currentFramebufferIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(device->presentQueue, &presentInfo);
}

void GraphicsPipeline::initViewportScissor(VkCommandBuffer_T* commandBuffer, const VkExtent2D* extent) {
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent->width);
	viewport.height = static_cast<float>(extent->height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = *extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void GraphicsPipeline::teardown(VkDevice_T* logicalDevice) {
	if (commandPool != nullptr) {
		commandPool->teardown(logicalDevice);
		delete commandPool;
		commandPool = nullptr;
	}
	if (syncObject != nullptr) {
		syncObject->teardown(logicalDevice);
		delete syncObject;
		syncObject = nullptr;
	}
	if (framebuffers != nullptr) {
		for (uint32_t i = 0; i < framebufferCount; i++) {
			vkDestroyFramebuffer(logicalDevice, framebuffers[i], nullptr);
		}
		delete[] framebuffers;
		framebuffers = nullptr;
	}
	if (pipeline != nullptr) {
		vkDestroyPipeline(logicalDevice, pipeline, nullptr);
		pipeline = nullptr;
	}
	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
		pipelineLayout = nullptr;
	}
	if (renderPass != nullptr) {
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
		renderPass = nullptr;
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
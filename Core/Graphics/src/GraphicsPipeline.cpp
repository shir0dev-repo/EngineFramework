#include "../GraphicsPipeline.h"
#include "../../Vulkan/Util/QueueFamilyIndices.h"
#include "../../Vulkan/VulkanSwapChain.h"
#include "../../Vulkan/VulkanDevice.h"
#include "../../Vulkan/VulkanCommandPool.h"
#include "../GraphicsSyncObject.h"
#include "../Frame.h"

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
	setupCommandBuffers(device, surface);
	setupFramebuffers(device->logicalDevice, swapChain);
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

	const VkExtent2D* const extent = swapChain->getExtents();

	for (uint32_t i = 0; i < MAX_FRAMEBUFFERS; i++) {
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

		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffer[i]->frameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void GraphicsPipeline::setupCommandBuffers(const VulkanDevice* const device, VkSurfaceKHR_T* surface) {
	const QueueFamilyIndices* const indices = device->getQueueFamilyIndices();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices->graphicsFamily.index;

	if (vkCreateCommandPool(device->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMEBUFFERS;
	std::vector<VkCommandBuffer> buffers(MAX_FRAMEBUFFERS);

	if (vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	framebuffer = new Frame*[MAX_FRAMEBUFFERS];
	for (uint32_t i = 0; i < MAX_FRAMEBUFFERS; i++) {
		framebuffer[i] = new Frame();
		framebuffer[i]->setup(device->logicalDevice, buffers[i]);
	}
}

void GraphicsPipeline::render(const VulkanDevice* const device, const VulkanSwapChain* const swapChain) {
	Frame* currentFrame = framebuffer[currentFramebufferIndex];
	initFrame(device, swapChain, currentFrame);

	beginCommandBuffer(currentFrame->commandBuffer);
	beginRenderPass(currentFrame->commandBuffer, swapChain);
	addRenderCommmand(currentFrame->commandBuffer);
	finishRenderPass(currentFrame->commandBuffer);
	finishCommandBuffer(currentFrame->commandBuffer);
	
	submitRender(currentFrame, device, swapChain);
	presentRender(currentFrame, device, swapChain);
	
	currentFramebufferIndex = (currentFramebufferIndex + 1) % MAX_FRAMEBUFFERS;
}

void GraphicsPipeline::initFrame(const VulkanDevice* device, const VulkanSwapChain* const swapChain, Frame* currentFrame) {
	currentFrame->syncObject->wait(device->logicalDevice);

	currentFrame->syncObject->reset(device->logicalDevice);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device->logicalDevice, swapChain->getSwapChain(), UINT64_MAX, currentFrame->syncObject->imageAvailableSemaphore,
		VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(currentFrame->commandBuffer, 0);
}

void GraphicsPipeline::beginCommandBuffer(VkCommandBuffer_T* commandBuffer) {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
}

void GraphicsPipeline::beginRenderPass(VkCommandBuffer_T* commandBuffer, const VulkanSwapChain* const swapChain) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = this->renderPass;
	renderPassInfo.framebuffer = framebuffer[currentFramebufferIndex]->frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = *swapChain->getExtents();

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
	initViewportScissor(commandBuffer, swapChain->getExtents());
}

void GraphicsPipeline::addRenderCommmand(VkCommandBuffer_T* commandBuffer) {
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void GraphicsPipeline::finishRenderPass(VkCommandBuffer_T* commandBuffer) {
	vkCmdEndRenderPass(commandBuffer);
}

void GraphicsPipeline::finishCommandBuffer(VkCommandBuffer_T* commandBuffer) {
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void GraphicsPipeline::submitRender(Frame* currentFrame, const VulkanDevice* const device, const VulkanSwapChain* const swapChain) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { currentFrame->syncObject->imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &currentFrame->commandBuffer;
	VkSemaphore signalSemaphores[] = { currentFrame->syncObject->renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, currentFrame->syncObject->inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}
}

void GraphicsPipeline::presentRender(Frame* currentFrame, const VulkanDevice* const device, const VulkanSwapChain* const swapChain) {

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	
	VkSemaphore signalSemaphores[] = { currentFrame->syncObject->renderFinishedSemaphore };
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
	if (framebuffer != nullptr) {
		for (uint32_t i = 0; i < MAX_FRAMEBUFFERS; i++) {
			framebuffer[i]->teardown(logicalDevice);
		}
		delete[] framebuffer;
	}
	if (commandPool != nullptr) {
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		commandPool = nullptr;
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
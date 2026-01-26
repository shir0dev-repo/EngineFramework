#include "../GraphicsPipeline.h"
#include "../Shader/Vertex.h"
#include "../RenderCommand.h"

#include "../RenderPipelineInfo.h"
#include "../Util/RenderUtils.h"
#include "../../Vulkan/Util/QueueFamilyIndices.h"
#include "../../Vulkan/VulkanSwapChain.h"
#include "../../Vulkan/VulkanDevice.h"
#include "../GraphicsSyncObject.h"
#include "../Frame.h"

#include <vulkan/vulkan.h>
#include <fstream>
#include <vector>

static std::vector<RenderCommand*> renderCmds;

GraphicsPipeline* GraphicsPipeline::instance = nullptr;

GraphicsPipeline* const GraphicsPipeline::getInstance() {
	return instance;
}

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

void GraphicsPipeline::onWindowResized(GraphicsPipeline* pipelineInstance, GLFWwindow* window, int width, int height) {
	pipelineInstance->frameBufferResized = true;
}

void GraphicsPipeline::setup(const VulkanDevice* const device, VulkanSwapChain* const swapChain, VkSurfaceKHR_T* surface) {
	instance = this;
	this->swapChain = swapChain;

	setupRenderPass(device->logicalDevice);
	setupPipelineLayout(device->logicalDevice);
	setupCommandPool(device, surface);
	setupVertexBuffer(device);

	uint32_t bufferCount = swapChain->getSwapChainImageCount();
	VkCommandBuffer_T** commandBuffers = nullptr;
	allocCommandBuffers(device->logicalDevice, bufferCount, commandBuffers);
	setupFramebuffers(device->logicalDevice, bufferCount, commandBuffers);
	delete[] commandBuffers;
}

void GraphicsPipeline::setupRenderPass(VkDevice_T* logicalDevice) {
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

void GraphicsPipeline::setupPipelineLayout(VkDevice_T* logicalDevice) {
	char* vertexContent = nullptr;
	char* fragmentContent = nullptr;

	uint32_t vertexSize, fragmentSize;

	readFile("Assets/Shaders/vert.spv", vertexContent, vertexSize);
	readFile("Assets/Shaders/frag.spv", fragmentContent, fragmentSize);

	VkShaderModule vertexShaderModule = createShaderModule(logicalDevice, vertexContent, vertexSize);
	VkShaderModule fragmentShaderModule = createShaderModule(logicalDevice, fragmentContent, fragmentSize);

	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};

	RenderPipelineInfo::makePipelineVertexShaderStateCreateInfo(vertexShaderModule, vertexShaderCreateInfo);
	RenderPipelineInfo::makePipelineFragmentShaderStateCreateInfo(fragmentShaderModule, fragmentShaderCreateInfo);
	
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	RenderPipelineInfo::makePipelineDynamicStateCreateInfo(dynamicState);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	RenderPipelineInfo::makePipelineVertexInputStateCreateInfo(vertexInputInfo);
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	RenderPipelineInfo::makePipelineInputAssemblyStateCreateInfo(inputAssembly);
	
	const VkExtent2D* const extent = swapChain->getExtents();
	VkViewport viewport{};
	VkRect2D scissor = {};
	RenderUtils::makeViewport(extent, viewport);
	RenderUtils::makeScissorRect(extent, scissor);
	
	VkPipelineViewportStateCreateInfo viewportState = {};
	RenderPipelineInfo::makePipelineViewportStateCreateInfo(&viewport, &scissor, viewportState);

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	RenderPipelineInfo::makePipelineRasterizationStateCreateInfo(rasterizer);
	
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	RenderPipelineInfo::makePipelineMultisampleStateCreateInfo(multisampling);
	
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	RenderPipelineInfo::makePipelineColorBlendAttachmentState(colorBlendAttachment);
	
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	RenderPipelineInfo::makePipelineColorBlendStateCreateInfo(&colorBlendAttachment, 1, colorBlending);

	RenderPipelineInfo::createPipelineLayout(logicalDevice, &this->pipelineLayout);
	
	GraphicsPipelineCreateParams createParams = {};
	createParams = RenderPipelineInfo::makeGraphicsPipelineCreateParams(shaderStages, &dynamicState, &vertexInputInfo, &inputAssembly, &viewportState, &rasterizer,
		&multisampling, &colorBlending, this->pipelineLayout, this->renderPass);
	
	RenderPipelineInfo::createGraphicsPipelines(logicalDevice, createParams, this->pipeline);

	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
}

void GraphicsPipeline::setupCommandPool(const VulkanDevice* const device, VkSurfaceKHR_T* surface) {
	const QueueFamilyIndices* const indices = device->getQueueFamilyIndices();

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = indices->graphicsFamily.index;

	if (vkCreateCommandPool(device->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

void GraphicsPipeline::setupVertexBuffer(const VulkanDevice* const device) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	
}

void GraphicsPipeline::allocCommandBuffers(VkDevice_T* logicalDevice, const uint32_t& count, VkCommandBuffer_T**& outBuffers) {
	if (outBuffers == nullptr) {
		outBuffers = new VkCommandBuffer_T * [count];
	}

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = count;
	std::vector<VkCommandBuffer> buffers(MAX_FRAMEBUFFERS);

	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, outBuffers) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void GraphicsPipeline::setupFramebuffers(VkDevice_T* logicalDevice, const uint32_t& bufferCount, VkCommandBuffer_T** commandBuffers) {
	Frame* const* const framebuffer = swapChain->getFramebuffer();
	const VkExtent2D* const extent = swapChain->getExtents();

	for (uint32_t i = 0; i < bufferCount; i++) {
		Frame* const frame = framebuffer[i];
		frame->commandBuffer = commandBuffers[i];
		frame->setupBuffer(logicalDevice, extent, this->renderPass);
	}
}

void GraphicsPipeline::render(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	Frame* currentFrame = swapChain->getCurrentFrame();
	bool shouldRender = initFrame(device, currentFrame, surface, window);
	if (!shouldRender) {
		return;
	}

	beginCommandBuffer(currentFrame->commandBuffer);
	beginRenderPass(currentFrame->commandBuffer);
	iterateRenderCommands(currentFrame->commandBuffer);
	//addRenderCommmand(currentFrame->commandBuffer);
	finishRenderPass(currentFrame->commandBuffer);
	finishCommandBuffer(currentFrame->commandBuffer);
	
	submitRender(device, currentFrame);
	presentRender(device, currentFrame, surface, window);
}

bool GraphicsPipeline::initFrame(const VulkanDevice* device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	currentFrame->syncObject->wait(device->logicalDevice);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device->logicalDevice, swapChain->getSwapChain(), UINT64_MAX,
		currentFrame->syncObject->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (swapChain->getCurrentFrameIndex() != imageIndex) {

	}
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		swapChain->recreate(device, renderPass, surface, window);
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}
	else {
		currentFrame->syncObject->reset(device->logicalDevice);
		vkResetCommandBuffer(currentFrame->commandBuffer, 0);
		return true;
	}
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

void GraphicsPipeline::beginRenderPass(VkCommandBuffer_T* commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = this->renderPass;
	renderPassInfo.framebuffer = swapChain->getCurrentFrame()->frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = *swapChain->getExtents();

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
	initViewportScissor(commandBuffer, swapChain->getExtents());
}

void GraphicsPipeline::addRenderCommand(GPUBuffer* buffers, uint32_t bufferCount, uint32_t vertexCount, uint32_t indexCount) {
	RenderCommand* cmd = RenderCommand::create(buffers, bufferCount, vertexCount, indexCount);
	renderCmds.push_back(cmd);
}

void GraphicsPipeline::addRenderCommmand(VkCommandBuffer_T* commandBuffer) {
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void GraphicsPipeline::iterateRenderCommands(VkCommandBuffer_T* commandBuffer) {
	for (uint32_t i = 0; i < renderCmds.size(); i++) {
		RenderCommand* cmd = renderCmds.data()[i];
		cmd->execute(commandBuffer);
		delete cmd;
	}

	renderCmds.clear();
}

void GraphicsPipeline::finishRenderPass(VkCommandBuffer_T* commandBuffer) {
	vkCmdEndRenderPass(commandBuffer);
}

void GraphicsPipeline::finishCommandBuffer(VkCommandBuffer_T* commandBuffer) {
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void GraphicsPipeline::submitRender(const VulkanDevice* const device, Frame* currentFrame) {
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

void GraphicsPipeline::presentRender(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window) {

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	
	VkSemaphore signalSemaphores[] = { currentFrame->syncObject->renderFinishedSemaphore };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	uint32_t currentFrameIndex = swapChain->getCurrentFrameIndex();
	presentInfo.pImageIndices = &currentFrameIndex;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(device->presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
		frameBufferResized = false;
		swapChain->recreate(device, renderPass, surface, window);
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image!");
	}
	else {
		swapChain->incrementCurrentFrame();
	}
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
	if (commandPool != nullptr) {
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		commandPool = nullptr;
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
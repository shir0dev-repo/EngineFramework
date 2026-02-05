#include "../RenderPass.h"
#include "../GraphicsPipeline.h"
#include "../../../Vulkan/VulkanSwapChain.h"
#include "../../../Vulkan/VulkanDevice.h"
#include "../../../Vulkan/VulkanInstance.h"
#include "../../Frame.h"
#include "../../GraphicsSyncObject.h"
#include "../../../Structure/linkedList.h"
#include "../../../Vulkan/Util/QueueFamilyIndices.h"

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

static std::vector<GraphicsPipeline*> pipelines;

Frame* const RenderPass::getCurrentFrame() const {
	return swapChain->getCurrentFrame();
}

void RenderPass::setup(VulkanInstance* instance) {
	this->graphicsPipelines = new linkedList<GraphicsPipeline*>();
	this->swapChain = instance->swapChain;
	
	setupRenderPass(instance->device);
	setupCommandPool(instance);
}

void RenderPass::setupCommandPool(VulkanInstance* vkInstance) {
	QueueFamilyIndices familyIndices;
	QueueFamilyIndices::findQueueFamilies(vkInstance->device->physicalDevice, vkInstance->surface, familyIndices);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = familyIndices.graphicsFamily.index;

	if (vkCreateCommandPool(vkInstance->device->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

void RenderPass::setupRenderPass(const VulkanDevice* const device) {
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

	if (vkCreateRenderPass(device->logicalDevice, &createInfo, nullptr, &this->vkRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void RenderPass::setupFramebuffers(const VulkanDevice* const device) {

}

GraphicsPipeline* const RenderPass::createPipeline(const VulkanDevice* const device, Shader* vertex, Shader* fragment, bool isOpaque) {
	GraphicsPipeline* pipeline = new GraphicsPipeline();
	pipeline->setup(device, vertex, fragment, this->vkRenderPass);
	graphicsPipelines->add(pipeline);

	return pipeline;
}

void RenderPass::removePipeline(const VulkanDevice* const device, GraphicsPipeline* pipeline) {
	if (graphicsPipelines->remove(pipeline)) {
		pipeline->teardown(device);
	}
}

void RenderPass::onRender(VulkanInstance* const vulkanInstance, GLFWwindow* window) {
	Frame* currentFrame = getCurrentFrame();

	bool shouldRender = beginFrame(vulkanInstance->device, currentFrame, vulkanInstance->surface, window);
	
	if (shouldRender == false) {
		return;
	}

	beginCommandBuffer(currentFrame->commandBuffer);
	beginPass(currentFrame->commandBuffer);

	for (auto& pipeline : pipelines) {
		beginPipeline(currentFrame->commandBuffer, pipeline);
		renderPipeline(currentFrame->commandBuffer, pipeline);
		finalizePipeline(currentFrame->commandBuffer, pipeline);
	}

	finalizePass(currentFrame->commandBuffer);
	submitRender(vulkanInstance->device, currentFrame);
	presentRender(vulkanInstance->device, currentFrame, vulkanInstance->surface, window);
}

bool RenderPass::beginFrame(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	currentFrame->syncObject->wait(device->logicalDevice);

	uint32_t imageIndex;
	VkResult acquireResult = vkAcquireNextImageKHR(device->logicalDevice, swapChain->getSwapChain(), UINT64_MAX,
		currentFrame->syncObject->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
		swapChain->recreate(device, vkRenderPass, surface, window);
		return false;
	}
	else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}
	else {
		currentFrame->syncObject->reset(device->logicalDevice);
		vkResetCommandBuffer(currentFrame->commandBuffer, 0);
		return true;
	}
}

void RenderPass::beginCommandBuffer(VkCommandBuffer_T* commandBuffer) {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
}

void RenderPass::beginPass(VkCommandBuffer_T* commandBuffer) {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vkRenderPass;
	renderPassInfo.framebuffer = swapChain->getCurrentFrame()->frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = *swapChain->getExtents();

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::beginPipeline(VkCommandBuffer_T* commandBuffer, GraphicsPipeline* pipeline) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
}

void RenderPass::renderPipeline(VkCommandBuffer_T* commandBuffer, GraphicsPipeline* pipeline) {
	pipeline->executeRenderCommands(commandBuffer);
}

void RenderPass::finalizePipeline(VkCommandBuffer_T* commandBuffer, GraphicsPipeline* pipeline) {

}

void RenderPass::finalizePass(VkCommandBuffer_T* commandBuffer) {
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void RenderPass::submitRender(const VulkanDevice* const device, Frame* currentFrame) {
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

void RenderPass::presentRender(const VulkanDevice* const device, Frame* currentFrame, VkSurfaceKHR_T* surface, GLFWwindow* window) {
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
		swapChain->recreate(device, vkRenderPass, surface, window);
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image!");
	}
	else {
		swapChain->incrementCurrentFrame();
	}
}

void RenderPass::teardown() {

}
#include "../Renderer.h"
#include "../../Pipeline/GraphicsPipeline.h"
#include "../../Frame.h"
#include "../../GraphicsSyncObject.h"
#include "../../Shader/PipelineShader.h"
#include "../../Uniform/GPUCameraData.h"
#include "../../Uniform/UniformBuffer.h"
#include "../../../Vulkan/VulkanInstance.h"
#include "../../../Vulkan/VulkanDevice.h"
#include "../../../Vulkan/VulkanSwapChain.h"
#include "../../../Structure/linkedList.h"
#include "../../../AppWindow.h"

#include <shml/quat.hpp>
#include <shml/mathutil.hpp>
#include <shml/matrix4f.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

Renderer* const Renderer::getInstance() {
	static Renderer* instance = nullptr;
	if (instance == nullptr) {
		instance = new Renderer();
	}
	return instance;
}

VkCommandPool_T* const Renderer::getCurrentCommandPool() const { return this->vkCommandPool; }
VkRenderPass_T* const Renderer::getRenderPass() const { return vkRenderPass; }
VkDescriptorSetLayout_T* const Renderer::getGlobalDescriptorSetLayout() const { return globalDescriptorLayout; }

void Renderer::notifyFramebufferResized() {
	frameBufferResized = true;
}

void Renderer::setup(VulkanInstance* vkInstance) {
	this->swapChain = vkInstance->swapChain; // cached reference
	this->numFrames = swapChain->getSwapChainImageCount();

	setupRenderPass(vkInstance->device);
	setupCommandPool(vkInstance->device);
	setupCommandBuffers(vkInstance->device);
	setupFramebuffers(vkInstance->device);
	setupSyncs(vkInstance->device);

	setupGlobalUniforms(vkInstance);
	setupGlobalDescriptorLayout(vkInstance);
	setupGlobalDescriptorPool(vkInstance);
	setupGlobalDescriptorSets(vkInstance);

	this->graphicsPipelines = new linkedList<GraphicsPipeline*>();
}

void Renderer::setupRenderPass(const VulkanDevice* const device) {
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

void Renderer::setupCommandPool(const VulkanDevice* const device) {
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = device->graphicsQueueFamilyIndex;

	if (vkCreateCommandPool(device->logicalDevice, &poolInfo, nullptr, &this->vkCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

void Renderer::setupCommandBuffers(const VulkanDevice* const device) {
	this->vkCommandBuffers = new VkCommandBuffer_T* [numFrames] { nullptr };
	
	VkCommandBufferAllocateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	createInfo.commandBufferCount = numFrames;
	createInfo.commandPool = this->vkCommandPool;
	createInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	if (vkAllocateCommandBuffers(device->logicalDevice, &createInfo, this->vkCommandBuffers) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void Renderer::setupFramebuffers(const VulkanDevice* const device) {
	const VkExtent2D* extent = swapChain->getExtents();
	
	this->vkFramebuffers = new VkFramebuffer_T* [numFrames] { nullptr };
	
	for (uint32_t i = 0; i < numFrames; i++) {
		VkImageView_T* imageView = swapChain->getImageView(i);
		
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vkRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &imageView;
		framebufferInfo.width = extent->width;
		framebufferInfo.height = extent->height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device->logicalDevice, &framebufferInfo, nullptr, &vkFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void Renderer::setupSyncs(const VulkanDevice* const device) {
	uint32_t numSyncs = swapChain->getSwapChainImageCount();
	this->syncObjects = new GraphicsSyncObject[numSyncs];
	
	for (uint32_t i = 0; i < numSyncs; i++) {
		syncObjects[i] = {};
		syncObjects[i].setup(device->logicalDevice);
	}
}

void Renderer::setupGlobalUniforms(const VulkanInstance* const instance) {
	VkDeviceSize bufferSize = sizeof(GPUCameraData);
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memoryUsage = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	this->globalBuffers = new UniformBuffer* [numFrames];
	this->mappedGlobalBuffers = new void* [numFrames] { nullptr };

	for (uint32_t i = 0; i < numFrames; i++) {
		this->globalBuffers[i] = UniformBuffer::create(instance->device, bufferSize, usage, memoryUsage);
		vkMapMemory(instance->device->logicalDevice, globalBuffers[i]->vkMemory, 0, bufferSize, 0, &mappedGlobalBuffers[i]);
	}
}

void Renderer::setupGlobalDescriptorLayout(const VulkanInstance* const instance) {
	VkDescriptorSetLayoutBinding uboBinding = {};
	uboBinding.binding = 0;
	uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	
	VkDescriptorSetLayoutCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = 1;
	createInfo.pBindings = &uboBinding;
	if (vkCreateDescriptorSetLayout(instance->device->logicalDevice, &createInfo, nullptr, &this->globalDescriptorLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor layout for global uniform buffer!");
	}
}

void Renderer::setupGlobalDescriptorPool(const VulkanInstance* const instance) {
	std::vector<VkDescriptorPoolSize> sizes = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 } };
	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.maxSets = 10;
	createInfo.poolSizeCount = sizes.size();
	createInfo.pPoolSizes = sizes.data();
	
	if (vkCreateDescriptorPool(instance->device->logicalDevice, &createInfo, nullptr, &this->globalDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Could not create descriptor pool for global descriptors!");
	}
}

void Renderer::setupGlobalDescriptorSets(const VulkanInstance* const instance) {
	globalDescriptorSets = new VkDescriptorSet_T* [numFrames] { nullptr };

	for (uint32_t i = 0; i < swapChain->getSwapChainImageCount(); i++) {
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = globalDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &globalDescriptorLayout;

		if (vkAllocateDescriptorSets(instance->device->logicalDevice, &allocInfo, &(globalDescriptorSets[i])) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor set for global descriptor!");
		}

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = globalBuffers[i]->vkBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(GPUCameraData);

		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.dstBinding = 0;
		if (globalDescriptorSets) {
			setWrite.dstSet = &(*globalDescriptorSets[i]);
		}
		setWrite.descriptorCount = 1;
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(instance->device->logicalDevice, 1, &setWrite, 0, nullptr);
	}
}

void Renderer::addPipeline(PipelineShader* pipelineShader) {
	GraphicsPipeline* pipeline = new GraphicsPipeline();

	pipeline->setup(VulkanInstance::getInstance(), this, pipelineShader->vertexModule, pipelineShader->fragmentModule);
	graphicsPipelines->add(pipeline);
}

GraphicsPipeline* const Renderer::getPipeline(const PipelineShader* pipelineShader) {
	if (pipelineShader == nullptr) {
		return (*graphicsPipelines)[0];
	}
}

void Renderer::render(VulkanInstance* instance, GLFWwindow* window) {
	bool shouldRender = beginFrame(instance->device);
	if (!shouldRender) {
		handleInvalidSwapchain(instance, window);
		return;
	}

	beginCommandBufferForCurrentFrame();
	beginRenderPassForCurrentFrame();
	initViewportScissorForCurrentFrame();

	GraphicsPipeline* currentPipeline = nullptr;
	for (uint32_t i = 0; i < graphicsPipelines->size(); i++) {
		auto pipeline = (*graphicsPipelines)[i];
		if (currentPipeline != pipeline) {
			beginPipeline(pipeline);
		}

		executePipeline(pipeline);
		finalizePipeline(pipeline);
	}

	finalizeRenderPassForCurrentFrame();
	finalizeCommandBufferForCurrentFrame();
	updateGlobalBuffer(instance->device);

	submitRender(instance->device);
	bool successfullyPresented = presentRender(instance->device);

	if (!successfullyPresented) {
		handleInvalidSwapchain(instance, window);
	}
	else {
		currentFrame = (currentFrame + 1) % swapChain->getSwapChainImageCount();
	}
}

bool Renderer::beginFrame(const VulkanDevice* const device) {
	auto currentSync = syncObjects[currentFrame];
	currentSync.wait(device->logicalDevice);
	
	uint32_t imageIndex;
	VkResult acquireResult = vkAcquireNextImageKHR(device->logicalDevice, swapChain->getSwapChain(), UINT64_MAX,
		currentSync.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
		frameBufferResized = false;
		return false;
	}
	else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}
	else {
		currentSync.reset(device->logicalDevice);
		vkResetCommandBuffer(vkCommandBuffers[currentFrame], 0);
		return true;
	}
}

void Renderer::beginCommandBufferForCurrentFrame() {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(vkCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
}

void Renderer::updateGlobalBuffer(const VulkanDevice* const device) {
	static AppWindow* windowInstance = nullptr;
	static float dt = 0;
	
	if (windowInstance == nullptr) {
		windowInstance = AppWindow::getInstance();
	}
	
	GPUCameraData cameraData = {};
	cameraData.projection = shml::matrix4f::IDENTITY;
	const float nearPlane = 0.03f, farPlane = 100.0f;

	float aspect = windowInstance->Width / windowInstance->Height;
	float fov = 60.0f;
	float scale = tan(fov * 0.5f * shml::DEG2RAD);
	cameraData.projection(0, 0) = 1.0f / (aspect * scale);
	cameraData.projection(1, 1) = 1.0f / -scale;
	cameraData.projection(2, 2) = farPlane / (farPlane - nearPlane);
	cameraData.projection(2, 3) = (-(farPlane * nearPlane) / (farPlane - nearPlane));
	cameraData.projection(3, 2) = 1;
	cameraData.projection(3, 3) = 0;
	cameraData.projection.transpose();
	
	cameraData.view = shml::matrix4f::IDENTITY;
	cameraData.view.setPosition({ 0, 0.0f, 0 });
	//cameraData.view.setRotation(shml::quat(0, dt, 0));
	cameraData.view.invert();
	cameraData.view.transpose();

	memcpy(mappedGlobalBuffers[currentFrame], &cameraData, sizeof(GPUCameraData));

	dt += 0.02f;
}

void Renderer::bindGlobalDescriptors(GraphicsPipeline* pipeline) {

}

void Renderer::beginRenderPassForCurrentFrame() {
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vkRenderPass;
	renderPassInfo.framebuffer = vkFramebuffers[currentFrame];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = *swapChain->getExtents();

	VkClearValue clearColor = { {{ 0.0f, 0.0f, 0.0f, 1.0f }} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(vkCommandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::initViewportScissorForCurrentFrame() {
	VkViewport vp = {};
	const VkExtent2D* extents = swapChain->getExtents();

	vp.minDepth = 0;
	vp.maxDepth = 1.0f;
	vp.x = 0;
	vp.y = 0;
	vp.height = extents->height;
	vp.width = extents->width;
	vkCmdSetViewport(vkCommandBuffers[currentFrame], 0, 1, &vp);
	
	VkRect2D scissor = {};
	scissor.extent = *extents;
	scissor.offset = { 0, 0 };
	vkCmdSetScissor(vkCommandBuffers[currentFrame], 0, 1, &scissor);
}

void Renderer::beginPipeline(GraphicsPipeline* pipeline) {
	vkCmdBindPipeline(vkCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
	std::vector<VkDescriptorSet> toBind = {};
	
	vkCmdBindDescriptorSets(vkCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1,
		&globalDescriptorSets[currentFrame], 0, nullptr);

	pipeline->bindDescriptorSets(vkCommandBuffers[currentFrame], currentFrame);
}

void Renderer::executePipeline(GraphicsPipeline* pipeline) {
	pipeline->executeRenderCommands(vkCommandBuffers[currentFrame]);
}

void Renderer::finalizePipeline(GraphicsPipeline* pipeline) {

}

void Renderer::finalizeRenderPassForCurrentFrame() {
	vkCmdEndRenderPass(vkCommandBuffers[currentFrame]);
}

void Renderer::finalizeCommandBufferForCurrentFrame() {
	if (vkEndCommandBuffer(vkCommandBuffers[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void Renderer::submitRender(const VulkanDevice* const device) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	GraphicsSyncObject currentSync = syncObjects[currentFrame];
	VkSemaphore waitSemaphores[] = { currentSync.imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffers[currentFrame];
	VkSemaphore signalSemaphores[] = { currentSync.renderFinishedSemaphore};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, currentSync.inFlightFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}
}

bool Renderer::presentRender(const VulkanDevice* const device) {
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore signalSemaphores[] = { syncObjects[currentFrame].renderFinishedSemaphore};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain->getSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	
	presentInfo.pImageIndices = &currentFrame;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(device->presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
		frameBufferResized = false;
		return false;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image!");
	}
	else {
		return true;
	}
}

void Renderer::handleInvalidSwapchain(const VulkanInstance* const instance, GLFWwindow* window) {
	this->swapChain->recreate(instance->device, vkRenderPass, instance->surface, window);

	for (uint32_t i = 0; i < numFrames; i++) {
		vkDestroyFramebuffer(instance->device->logicalDevice, vkFramebuffers[i], nullptr);
	}
	delete[] vkFramebuffers;
	vkFramebuffers = nullptr;

	numFrames = swapChain->getSwapChainImageCount();
	currentFrame = 0;
	setupFramebuffers(instance->device);
}

void Renderer::teardown(VkDevice_T* logicalDevice) {
	if (globalBuffers != nullptr) {
		for (uint32_t i = 0; i < swapChain->getSwapChainImageCount(); i++) {
			UniformBuffer::dispose(globalBuffers[i], logicalDevice);
		}
		delete[] globalBuffers;
		globalBuffers = nullptr;
	}
	if (globalDescriptorLayout != nullptr) {
		vkDestroyDescriptorSetLayout(logicalDevice, globalDescriptorLayout, nullptr);
		globalDescriptorLayout = nullptr;
	}
	if (globalDescriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDevice, globalDescriptorPool, nullptr);
		globalDescriptorPool = nullptr;
		if (globalDescriptorSets != nullptr);
		delete[] globalDescriptorSets;
		globalDescriptorSets = nullptr;
	}
	if (vkCommandPool != nullptr) {
		vkDestroyCommandPool(logicalDevice, vkCommandPool, nullptr);
		vkCommandPool = nullptr;
		if (vkCommandBuffers != nullptr) {
			delete[] vkCommandBuffers;
			vkCommandBuffers = nullptr;
		}
	}
	if (vkFramebuffers != nullptr) {
		for (uint32_t i = 0; i < swapChain->getSwapChainImageCount(); i++) {
			vkDestroyFramebuffer(logicalDevice, vkFramebuffers[i], nullptr);
		}
		delete[] vkFramebuffers;
		vkFramebuffers = nullptr;
	}
	if (syncObjects != nullptr) {
		for (uint32_t i = 0; i < swapChain->getSwapChainImageCount(); i++) {
			syncObjects[i].teardown(logicalDevice);
		}
		delete[] syncObjects;
		syncObjects = nullptr;
	}
	if (graphicsPipelines != nullptr) {
		for (uint32_t i = 0; i < graphicsPipelines->size(); i++) {
			(*graphicsPipelines)[i]->teardown(logicalDevice);
		}
	}
	if (vkRenderPass != nullptr) {
		vkDestroyRenderPass(logicalDevice, vkRenderPass, nullptr);
		vkRenderPass = nullptr;
	}
	if (mappedGlobalBuffers != nullptr) {
		delete[] mappedGlobalBuffers;
		mappedGlobalBuffers = nullptr;
	}
}
#include "../VulkanSwapChain.h"
#include "../Util/QueueFamilyIndices.h"
#include "../Util/SwapChainSupportDetails.h"
#include "../VulkanDevice.h"
#include "../../Graphics/Frame.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

static constexpr uint32_t clamp(uint32_t val, uint32_t min, uint32_t max) {
	uint32_t result = 0;
	if (val < min)
		result = min;
	else if (val > max)
		result = max;
	else
		result = val;
	return result;
}

VkImageView_T* const VulkanSwapChain::getImageView(uint32_t i) const {
	if (i < 0 || i > swapChainImageCount) {
		return nullptr;
	}
	else {
		return swapChainImageViews[i];
	}
}

void VulkanSwapChain::setup(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	this->supportDetails = new SwapChainSupportDetails();
	querySwapChainCapabilities(device->physicalDevice, surface, *this->supportDetails);

	createSwapChain(device, surface, window);
	createFramebuffers(device);
}

void VulkanSwapChain::recreate(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	vkDeviceWaitIdle(device->logicalDevice);

	cleanupSwapchain(device->logicalDevice, false);
	
	querySwapChainCapabilities(device->physicalDevice, surface, *this->supportDetails);

	createSwapChain(device, surface, window);
	recreateFramebuffers(device);
}

void VulkanSwapChain::createSwapChain(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	chooseSwapSurfaceFormat(supportDetails->supportedFormats, supportDetails->supportedFormatsCount);
	chooseSwapPresentMode(supportDetails->supportedPresentModes, supportDetails->supportedPresentModesCount);
	chooseSwapExtent(*supportDetails->capabilities, window);

	uint32_t imageCount = supportDetails->capabilities->minImageCount + 1;
	if (supportDetails->capabilities->maxImageCount > 0 && imageCount > supportDetails->capabilities->maxImageCount) {
		imageCount = supportDetails->capabilities->maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = selectedFormat->format;
	createInfo.imageColorSpace = selectedFormat->colorSpace;
	createInfo.imageExtent = *swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = {};
	QueueFamilyIndices::findQueueFamilies(device->physicalDevice, surface, indices);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.index, indices.presentFamily.index };

	if (indices.graphicsFamily.index != indices.presentFamily.index) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = supportDetails->capabilities->currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = selectedPresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device->logicalDevice, &createInfo, nullptr, &this->swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}
}



void VulkanSwapChain::createFramebuffers(const VulkanDevice* const device) {
	vkGetSwapchainImagesKHR(device->logicalDevice, swapChain, &this->swapChainImageCount, nullptr);
	
	this->framebuffer = new Frame * [swapChainImageCount] { nullptr };
	VkImage_T** images = new VkImage_T * [swapChainImageCount] { nullptr };
	
	size_t frameBufferCount(swapChainImageCount);
	vkGetSwapchainImagesKHR(device->logicalDevice, this->swapChain, &swapChainImageCount, images);
	for (uint32_t i = 0; i < frameBufferCount; i++) {
		Frame* frame = new Frame();
		frame->setupImageView(device->logicalDevice, selectedFormat->format, images[i]);

		framebuffer[i] = frame;
	}

	delete[] images;
}

void VulkanSwapChain::recreateFramebuffers(const VulkanDevice* const device) {
	for (uint32_t i = 0; i < swapChainImageCount; i++) {
		Frame* frame = framebuffer[i];
		frame->setupImageView(device->logicalDevice, selectedFormat->format, frame->image);
	}
}

void VulkanSwapChain::teardown(VkDevice_T* device) {
	if (framebuffer != nullptr) {
		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			framebuffer[i]->teardown(device, true);
		}
		delete[] framebuffer;
	}
	if (swapChainImageViews != nullptr) {
		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		delete[] swapChainImageViews;
		swapChainImageViews = nullptr;
	}
	if (swapChainImages != nullptr) {
		delete[] swapChainImages;
		swapChainImages = nullptr;
	}
	if (supportDetails != nullptr) {
		delete supportDetails;
		supportDetails = nullptr;
	}
	if (selectedFormat != nullptr) {
		delete selectedFormat;
		selectedFormat = nullptr;
	}
	if (swapExtent != nullptr) {
		delete swapExtent;
		swapExtent = nullptr;
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanSwapChain::cleanupSwapchain(VkDevice_T* logicalDevice, bool isFinalTeardown) {
	for (uint32_t i = 0; i < swapChainImageCount; i++) {
		framebuffer[i]->teardown(logicalDevice, isFinalTeardown);
	}

	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
	swapChain = nullptr;
}

void VulkanSwapChain::querySwapChainCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, SwapChainSupportDetails& supportDetails) {
	querySurfaceCapabilities(device, surface, supportDetails.capabilities);
	querySupportedSurfaceFormats(device, surface, supportDetails.supportedFormats, &supportDetails.supportedFormatsCount);
	querySupportedPresentModes(device, surface, supportDetails.supportedPresentModes, &supportDetails.supportedPresentModesCount);
}

void VulkanSwapChain::querySurfaceCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceCapabilitiesKHR*& outCapabilities) {
	if (outCapabilities != nullptr) {
		delete outCapabilities;
	}

	outCapabilities = new VkSurfaceCapabilitiesKHR();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, outCapabilities);
}

void VulkanSwapChain::querySupportedSurfaceFormats(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceFormatKHR*& outFormats, uint32_t* count) {
	if (outFormats != nullptr) {
		delete[] outFormats;
		outFormats = nullptr;
	}

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, nullptr);
	if (*count <= 0) {
		outFormats = nullptr;
		return;
	}

	outFormats = new VkSurfaceFormatKHR[*count];
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, outFormats);
}

void VulkanSwapChain::querySupportedPresentModes(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkPresentModeKHR*& outPresentModes, uint32_t* count) {
	if (outPresentModes != nullptr) {
		delete[] outPresentModes;
		outPresentModes = nullptr;
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, nullptr);
	if (*count <= 0) {
		outPresentModes = nullptr;
		return;
	}

	outPresentModes = new VkPresentModeKHR[*count];
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, outPresentModes);
}

void VulkanSwapChain::chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t count) {
	if (count <= 0) throw;

	for (uint32_t i = 0; i < count; i++) {
		VkSurfaceFormatKHR availableFormat = availableFormats[i];
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			selectedFormat = new VkSurfaceFormatKHR(availableFormat);
			return;
		}
	}

	selectedFormat = new VkSurfaceFormatKHR(availableFormats[0]);
	swapChainImageFormat = selectedFormat->format;
}

void VulkanSwapChain::chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t count) {
	if (count <= 0) throw;

	for (uint32_t i = 0; i < count; i++) {
		VkPresentModeKHR availablePresentMode = availablePresentModes[i];
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			selectedPresentMode = availablePresentMode;
			return;
		}
	}

	selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	if (capabilities.currentExtent.width != 0xFFFFFFFF) {
		swapExtent = new VkExtent2D(capabilities.currentExtent);
		return;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		VkExtent2D actualExtents = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height),
		};

		actualExtents.width = clamp(actualExtents.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtents.height = clamp(actualExtents.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		swapExtent = new VkExtent2D(actualExtents);
	}
}
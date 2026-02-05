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

VkImage_T* const VulkanSwapChain::getImage(uint32_t index) {
	if (index < 0 || index >= swapChainImageCount) {
		throw std::runtime_error("Invalid index for swap chain image!");
	}

	return this->swapChainImages[index];
}

VkImageView_T* const VulkanSwapChain::getImageView(uint32_t index) {
	if (index < 0 || index >= swapChainImageCount) {
		throw std::runtime_error("Invalid index for swap chain image view!");
	}

	return this->swapChainImageViews[index];
}

void VulkanSwapChain::setup(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	this->supportDetails = new SwapChainSupportDetails();
	querySwapChainCapabilities(device->physicalDevice, surface, *this->supportDetails);

	createSwapChain(device, surface, window);
	createImages(device->logicalDevice);
	createImageViews(device->logicalDevice);
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

	uint32_t gIndex = device->graphicsQueueFamilyIndex;
	uint32_t pIndex = device->presentQueueFamilyIndex;
	uint32_t queueFamilyIndices[] = { gIndex, pIndex };

	if (gIndex != pIndex) {
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

void VulkanSwapChain::createImages(VkDevice_T* logicalDevice) {
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &this->swapChainImageCount, nullptr);
	this->swapChainImages = new VkImage_T* [swapChainImageCount] { nullptr };
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, this->swapChainImages);
}
void VulkanSwapChain::createImageViews(VkDevice_T* logicalDevice) {
	this->swapChainImageViews = new VkImageView_T* [swapChainImageCount] { nullptr };

	for (uint32_t i = 0; i < this->swapChainImageCount; i++) {
		VkImage_T* image = swapChainImages[i];

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = selectedFormat->format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &this->swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}
}

void VulkanSwapChain::recreate(const VulkanDevice* const device, VkRenderPass_T* renderPass, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		if (glfwWindowShouldClose(window)) {
			return;
		}
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device->logicalDevice);

	cleanupSwapchain(device->logicalDevice, false);
	querySwapChainCapabilities(device->physicalDevice, surface, *this->supportDetails);
	createSwapChain(device, surface, window);
}

void VulkanSwapChain::teardown(VkDevice_T* device) {
	if (swapChainImageViews != nullptr) {
		for (uint32_t i = 0; i < swapChainImageCount; i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		delete[] swapChainImageViews;
		delete[] swapChainImages;
		swapChainImageViews = nullptr;
		swapChainImages = nullptr;
	}
	if (swapChain != nullptr) {
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = nullptr;
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
}

void VulkanSwapChain::cleanupSwapchain(VkDevice_T* logicalDevice, bool isFinalTeardown) {
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
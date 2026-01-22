#include "../VulkanDevice.h"
#include "../VulkanValidator.h"
#include "../VulkanSwapChain.h"
#include "../Util/QueueFamilyIndices.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <optional>
#include <limits>
#include <algorithm>

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//struct SwapChainSupportDetails {
//	VkSurfaceCapabilitiesKHR capabilities;
//	std::vector<VkSurfaceFormatKHR> formats;
//	std::vector<VkPresentModeKHR> presentModes;
//};
//
//SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface) {
//	SwapChainSupportDetails details = {};
//	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
//
//	uint32_t formatCount;
//	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
//
//	if (formatCount != 0) {
//		details.formats.resize(formatCount);
//		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
//	}
//
//	uint32_t presentModeCount;
//	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
//
//	if (presentModeCount != 0) {
//		details.presentModes.resize(presentModeCount);
//		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
//	}
//	return details;
//}
//
//VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
//	for (const auto& availableFormat : availableFormats) {
//		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//			return availableFormat;
//		}
//	}
//
//	return availableFormats[0];
//}
//
//VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
//	for (const auto& availablePresentMode : availablePresentModes) {
//		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//			return availablePresentMode;
//		}
//	}
//
//	return VK_PRESENT_MODE_FIFO_KHR;
//}
//
//VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
//	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
//		return capabilities.currentExtent;
//	}
//	else
//	{
//		int width, height;
//		glfwGetFramebufferSize(window, &width, &height);
//
//		VkExtent2D actualExtents = {
//			static_cast<uint32_t>(width),
//			static_cast<uint32_t>(height)
//		};
//
//		actualExtents.width = std::clamp(actualExtents.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
//		actualExtents.height = std::clamp(actualExtents.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
//
//		return actualExtents;
//	}
//}

//struct QueueFamilyIndices {
//	std::optional<uint32_t> graphicsFamily;
//	std::optional<uint32_t> presentFamily;
//
//	bool isComplete() const {
//		return graphicsFamily.has_value() && presentFamily.has_value();
//	}
//};
//
//static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR_T* surface) {
//	QueueFamilyIndices indices;
//	uint32_t queueFamilyCount = 0;
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//
//	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
//
//	int i = 0;
//	for (const auto& queueFamily : queueFamilies) {
//		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//			VkBool32 presentSupport = false;
//			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
//			if (presentSupport) {
//				indices.presentFamily = i;
//			}
//			indices.graphicsFamily = i;
//		}
//
//		if (indices.isComplete()) {
//			break;
//		}
//
//		i++;
//	}
//	return indices;
//}

static bool checkDeviceExtensionSupport(VkPhysicalDevice_T* device);
static uint32_t rateDeviceSuitability(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface);

void VulkanDevice::setup(VkInstance_T* instance, const VulkanValidator& validator, VkSurfaceKHR_T* surface, GLFWwindow* window) {
	pickPhysicalDevice(instance, surface);
	createLogicalDevice(validator, surface);
	createSwapChain(surface, window);
}
void VulkanDevice::teardown() {
	if (swapChain != nullptr) {
		swapChain->teardown(logicalDevice);
		delete swapChain;
		swapChain = nullptr;
	}

	vkDestroyDevice(logicalDevice, nullptr);
}

void VulkanDevice::pickPhysicalDevice(VkInstance_T* instance, VkSurfaceKHR_T* surface) {
	physicalDevice = VK_NULL_HANDLE;
	
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	
	if (deviceCount == 0) {
		throw std::runtime_error("No physical devices supporting Vulkan were found!");
	}
	
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		uint32_t score = rateDeviceSuitability(device, surface);
		candidates.insert(std::make_pair(score, device));
	}
	if (candidates.rbegin()->first > 0) {
		this->physicalDevice = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}
void VulkanDevice::createLogicalDevice(const VulkanValidator& validator, VkSurfaceKHR_T* surface) {
	QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(physicalDevice, surface);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.index, indices.presentFamily.index };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};

		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (validator.isValidationLayerEnabled) {
		deviceCreateInfo.enabledLayerCount = validator.numEnabledLayers;
		deviceCreateInfo.ppEnabledLayerNames = validator.getValidationLayerNames();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr, &this->logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.index, 0, &this->graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.index, 0, &this->presentQueue);
}

void VulkanDevice::createSwapChain(VkSurfaceKHR_T* surface, GLFWwindow* window) {
	SwapChainSupportDetails swapChainSupport = {};
	VulkanSwapChain::querySwapChainCapabilities(physicalDevice, surface, swapChainSupport);
	this->swapChain = new VulkanSwapChain();
	swapChain->setup(physicalDevice, logicalDevice, surface, window, swapChainSupport);
}

bool checkDeviceExtensionSupport(VkPhysicalDevice_T* device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

uint32_t rateDeviceSuitability(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	uint32_t score = 0;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	if (!extensionsSupported) {
		return 0;
	}

	SwapChainSupportDetails swapChainSupport = {};
	VulkanSwapChain::querySwapChainCapabilities(device, surface, swapChainSupport);
	bool swapChainAdequate = swapChainSupport.supportedFormats != nullptr && swapChainSupport.supportedPresentModes != nullptr;

	if (!deviceFeatures.geometryShader || !indices.isComplete() || !swapChainAdequate) {
		return 0;
	}

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	score += deviceProperties.limits.maxImageDimension2D;
	return score;
}
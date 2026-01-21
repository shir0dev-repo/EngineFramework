#include "../VulkanValidator.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <cstring>
#include <iostream>

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

void VulkanValidator::initialize() {
	if (!isValidationLayerEnabled) {
		isSupported = false;
		return;
	}

	vkEnumerateInstanceLayerProperties(&numAvailableLayers, nullptr);
	std::vector<VkLayerProperties> availableLayers(numAvailableLayers);
	vkEnumerateInstanceLayerProperties(&numAvailableLayers, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				numEnabledLayers++;
				break;
			}
		}

		if (!layerFound) {
			isSupported = false;
		}
	}

	isSupported = true;
}

const char* const* VulkanValidator::getValidationLayerNames() const {
	return validationLayers.data();
}

const char* const* VulkanValidator::getRequiredExtensions() {
	static std::vector<const char*> requiredGlfwExtensions;
	static uint32_t extensionsCount = 0;

	if (numExtensions > 0) {
		return requiredGlfwExtensions.data();
	}

	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&numExtensions);
	requiredGlfwExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + numExtensions);

	if (isValidationLayerEnabled) {
		requiredGlfwExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		numExtensions++;
	}
	extensionsCount = requiredGlfwExtensions.size();
	numExtensions = extensionsCount;

	return requiredGlfwExtensions.data();
}
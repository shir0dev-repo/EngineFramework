#include "../QueueFamilyIndices.h"

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

bool QueueFamilyIndices::isComplete() const {
	return graphicsFamily.exists && presentFamily.exists;
}

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface) {
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32_t i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = { i, true };
			}

			indices.graphicsFamily = { i, true };
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}
	return indices;
}
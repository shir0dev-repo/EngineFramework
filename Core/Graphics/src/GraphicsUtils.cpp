#include "../GraphicsUtils.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"
#include "../Pipeline/GraphicsPipeline.h"

#include <vulkan/vulkan.h>

bool GraphicsUtils::GetPhysicalDevice(VkPhysicalDevice_T*& physicalDevice) {
    static VkPhysicalDevice_T* device = nullptr;
    if (device == nullptr) {
        device = VulkanContext::getInstance()->getDevice()->getPhysicalDevice();
    }
    physicalDevice = device;
    return physicalDevice != nullptr;
}

bool GraphicsUtils::GetLogicalDevice(VkDevice_T*& logicalDevice) {
    static VkDevice_T* device = nullptr;
    if (device == nullptr) {
        device = VulkanContext::getInstance()->getDevice()->getLogicalDevice();
    }
    logicalDevice = device;
    return logicalDevice != nullptr;
}

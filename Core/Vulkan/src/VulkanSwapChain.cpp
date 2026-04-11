#include "../VulkanSwapChain.h"
#include "../VulkanDevice.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

// ---------------------------------------------------------------------------
// SwapChainSupportDetails — file-local; no longer a public header
// ---------------------------------------------------------------------------
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR* capabilities        = nullptr;
    VkSurfaceFormatKHR*       supportedFormats     = nullptr;
    VkPresentModeKHR*         supportedPresentModes = nullptr;
    uint32_t supportedFormatsCount    = 0;
    uint32_t supportedPresentModesCount = 0;

    ~SwapChainSupportDetails() {
        delete capabilities;
        delete[] supportedFormats;
        delete[] supportedPresentModes;
    }
};

static constexpr uint32_t clamp(uint32_t val, uint32_t minVal, uint32_t maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// ---------------------------------------------------------------------------
// Public static helpers
// ---------------------------------------------------------------------------
bool VulkanSwapChain::isSwapChainAdequate(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface) {
    SwapChainSupportDetails details{};
    querySwapChainCapabilities(device, surface, details);
    return details.supportedFormats != nullptr && details.supportedPresentModes != nullptr;
}

void VulkanSwapChain::querySwapChainCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface,
                                                  SwapChainSupportDetails& supportDetails) {
    querySurfaceCapabilities(device, surface, supportDetails.capabilities);
    querySupportedSurfaceFormats(device, surface, supportDetails.supportedFormats, &supportDetails.supportedFormatsCount);
    querySupportedPresentModes(device, surface, supportDetails.supportedPresentModes, &supportDetails.supportedPresentModesCount);
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------
VkImage_T* const VulkanSwapChain::getImage(uint32_t index) {
    if (index >= swapChainImageCount) throw std::runtime_error("Invalid index for swap chain image!");
    return swapChainImages[index];
}

VkImageView_T* const VulkanSwapChain::getImageView(uint32_t index) {
    if (index >= swapChainImageCount) throw std::runtime_error("Invalid index for swap chain image view!");
    return swapChainImageViews[index];
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
void VulkanSwapChain::setup(const VulkanDevice* device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
    this->supportDetails = new SwapChainSupportDetails();
    querySwapChainCapabilities(device->getPhysicalDevice(), surface, *this->supportDetails);
    createSwapChain(device, surface, window);
    createImages(device->getLogicalDevice());
    createImageViews(device->getLogicalDevice());
}

void VulkanSwapChain::recreate(const VulkanDevice* device, VkRenderPass_T* renderPass, VkSurfaceKHR_T* surface, GLFWwindow* window) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        if (glfwWindowShouldClose(window)) return;
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device->getLogicalDevice());

    cleanupSwapchain(device->getLogicalDevice(), false);
    querySwapChainCapabilities(device->getPhysicalDevice(), surface, *this->supportDetails);
    createSwapChain(device, surface, window);
    createImages(device->getLogicalDevice());
    createImageViews(device->getLogicalDevice());
}

void VulkanSwapChain::teardown(VkDevice_T* device) {
    if (swapChainImageViews != nullptr) {
        for (uint32_t i = 0; i < swapChainImageCount; i++) {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }
        delete[] swapChainImageViews;
        delete[] swapChainImages;
        swapChainImageViews = nullptr;
        swapChainImages     = nullptr;
    }
    if (swapChain != nullptr) {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        swapChain = nullptr;
    }
    delete supportDetails; supportDetails = nullptr;
    delete selectedFormat; selectedFormat = nullptr;
    delete swapExtent;     swapExtent     = nullptr;
}

// ---------------------------------------------------------------------------
// Private creation helpers
// ---------------------------------------------------------------------------
void VulkanSwapChain::createSwapChain(const VulkanDevice* device, VkSurfaceKHR_T* surface, GLFWwindow* window) {
    chooseSwapSurfaceFormat(supportDetails->supportedFormats, supportDetails->supportedFormatsCount);
    chooseSwapPresentMode(supportDetails->supportedPresentModes, supportDetails->supportedPresentModesCount);
    chooseSwapExtent(*supportDetails->capabilities, window);

    uint32_t imageCount = supportDetails->capabilities->minImageCount + 1;
    if (supportDetails->capabilities->maxImageCount > 0 && imageCount > supportDetails->capabilities->maxImageCount) {
        imageCount = supportDetails->capabilities->maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = selectedFormat->format;
    createInfo.imageColorSpace  = selectedFormat->colorSpace;
    createInfo.imageExtent      = *swapExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t gIndex = device->getGraphicsQueueFamilyIndex();
    uint32_t pIndex = device->getPresentQueueFamilyIndex();
    uint32_t queueFamilyIndices[] = { gIndex, pIndex };

    if (gIndex != pIndex) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = supportDetails->capabilities->currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = selectedPresentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->getLogicalDevice(), &createInfo, nullptr, &this->swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }
}

void VulkanSwapChain::createImages(VkDevice_T* logicalDevice) {
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, nullptr);
    swapChainImages = new VkImage_T*[swapChainImageCount]{ nullptr };
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages);
}

void VulkanSwapChain::createImageViews(VkDevice_T* logicalDevice) {
    swapChainImageViews = new VkImageView_T*[swapChainImageCount]{ nullptr };

    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image        = swapChainImages[i];
        createInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format       = selectedFormat->format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view!");
        }
    }
}

void VulkanSwapChain::cleanupSwapchain(VkDevice_T* logicalDevice, bool isFinalTeardown) {
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        vkDestroyImageView(logicalDevice, swapChainImageViews[i], nullptr);
    }
    delete[] swapChainImages;     swapChainImages     = nullptr;
    delete[] swapChainImageViews; swapChainImageViews = nullptr;
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
    swapChain = nullptr;
}

// ---------------------------------------------------------------------------
// Private query helpers
// ---------------------------------------------------------------------------
void VulkanSwapChain::querySurfaceCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface,
                                               VkSurfaceCapabilitiesKHR*& outCapabilities) {
    delete outCapabilities;
    outCapabilities = new VkSurfaceCapabilitiesKHR();
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, outCapabilities);
}

void VulkanSwapChain::querySupportedSurfaceFormats(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface,
                                                    VkSurfaceFormatKHR*& outFormats, uint32_t* count) {
    delete[] outFormats;
    outFormats = nullptr;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, nullptr);
    if (*count <= 0) return;
    outFormats = new VkSurfaceFormatKHR[*count];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, count, outFormats);
}

void VulkanSwapChain::querySupportedPresentModes(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface,
                                                  VkPresentModeKHR*& outPresentModes, uint32_t* count) {
    delete[] outPresentModes;
    outPresentModes = nullptr;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, nullptr);
    if (*count <= 0) return;
    outPresentModes = new VkPresentModeKHR[*count];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, count, outPresentModes);
}

void VulkanSwapChain::chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t count) {
    if (count <= 0) throw;
    for (uint32_t i = 0; i < count; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selectedFormat = new VkSurfaceFormatKHR(availableFormats[i]);
            return;
        }
    }
    selectedFormat = new VkSurfaceFormatKHR(availableFormats[0]);
}

void VulkanSwapChain::chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t count) {
    if (count <= 0) throw;
    for (uint32_t i = 0; i < count; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            selectedPresentMode = availablePresentModes[i];
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
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    VkExtent2D actual = {
        clamp(static_cast<uint32_t>(width),  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width),
        clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
    swapExtent = new VkExtent2D(actual);
}

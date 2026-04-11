#pragma once

typedef unsigned int uint32_t;

struct VulkanDevice;

struct GLFWwindow;

typedef enum VkPresentModeKHR;
typedef enum VkFormat;

struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkSurfaceKHR_T;
struct VkSwapchainKHR_T;
struct VkRenderPass_T;
struct VkImage_T;
struct VkFramebuffer_T;
struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;
struct VkExtent2D;
struct VkImageView_T;

/// <summary>Manages the Vulkan swap chain, images, and image views.</summary>
struct VulkanSwapChain {
    /// <summary>Checks whether a device/surface pair supports adequate swap chain capabilities.</summary>
    static bool isSwapChainAdequate(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface);

    /// <summary>Sets up the swap chain.</summary>
    void setup(const VulkanDevice* device, VkSurfaceKHR_T* surface, GLFWwindow* window);

    /// <summary>Recreates the swap chain after resize or invalidation.</summary>
    void recreate(const VulkanDevice* device, VkRenderPass_T* renderPass, VkSurfaceKHR_T* surface, GLFWwindow* window);

    /// <summary>Cleans up swap chain resources.</summary>
    void teardown(VkDevice_T* logicalDevice);

    VkImage_T* const    getImage(uint32_t index);
    VkImageView_T* const getImageView(uint32_t index);

    const VkSurfaceFormatKHR* const getFormat()              const { return selectedFormat; }
    const VkPresentModeKHR          getPresentMode()         const { return selectedPresentMode; }
    const VkExtent2D* const         getExtents()             const { return swapExtent; }
    VkSwapchainKHR_T* const         getSwapChain()           const { return swapChain; }
    const uint32_t                  getSwapChainImageCount() const { return swapChainImageCount; }

private:
    void createSwapChain(const VulkanDevice* device, VkSurfaceKHR_T* surface, GLFWwindow* window);
    void createImages(VkDevice_T* logicalDevice);
    void createImageViews(VkDevice_T* logicalDevice);
    void cleanupSwapchain(VkDevice_T* logicalDevice, bool isFinalTeardown);

    // Capability queries — internal only, SwapChainSupportDetails lives in .cpp
    static void querySwapChainCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, struct SwapChainSupportDetails& supportDetails);
    static void querySupportedSurfaceFormats(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceFormatKHR*& outFormats, uint32_t* count);
    static void querySupportedPresentModes(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkPresentModeKHR*& outPresentModes, uint32_t* count);
    static void querySurfaceCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceCapabilitiesKHR*& outCapabilities);

    void chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t count);
    void chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t count);
    void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    struct SwapChainSupportDetails* supportDetails = nullptr;

    VkImage_T**     swapChainImages     = nullptr;
    VkImageView_T** swapChainImageViews = nullptr;
    uint32_t        swapChainImageCount = 0;
    VkSwapchainKHR_T*   swapChain       = nullptr;
    VkSurfaceFormatKHR* selectedFormat  = nullptr;
    VkPresentModeKHR    selectedPresentMode;
    VkExtent2D*         swapExtent      = nullptr;
};

#pragma once

typedef unsigned int uint32_t;
typedef enum VkFormat;
typedef enum VkImageTiling;
typedef uint32_t VkFlags;
typedef VkFlags VkFormatFeatureFlags;

struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkInstance_T;
struct VkSurfaceKHR_T;
struct VkQueue_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkDebugUtilsMessengerCreateInfoEXT;
struct VkApplicationInfo;
struct VkInstanceCreateInfo;

struct GLFWwindow;
struct VulkanSwapChain;
struct VulkanDevice;

/// <summary>Core Vulkan context: owns the instance, device, queues, surface, and command pool.</summary>
struct VulkanContext {
    static VulkanContext* const getInstance();

    bool setup(GLFWwindow* window);
    void teardown();

    /// <summary>Returns the device facade for accessing hardware handles.</summary>
    const VulkanDevice* getDevice() const;
    /// <summary>Returns the swap chain.</summary>
    VulkanSwapChain* getSwapChain() const;
    /// <summary>Returns the window surface.</summary>
    VkSurfaceKHR_T* getSurface() const;

    VkFormat querySupportedFormats(VkFormat* candidates, uint32_t candidateCount,
                                   VkImageTiling tiling, VkFormatFeatureFlags features) const;

    bool beginSingleUseCommandBuffer(VkCommandBuffer_T** buffer) const;
    void endSingleUseCommandBuffer(VkCommandBuffer_T* buffer) const;

private:
    static VulkanContext* vulkanContext;

    // Validator state (inlined — no separate VulkanValidator class)
    #define VALIDATION_LAYER_ENABLED 1
    bool isValidatorSupported = false;
    uint32_t numAvailableValidationLayers = 0;
    uint32_t numEnabledValidationLayers = 0;
    uint32_t numGLFWExtensions = 0;
    VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;

    // Core Vulkan handles (all private)
    VkInstance_T* instance = nullptr;
    VkSurfaceKHR_T* surface = nullptr;
    VkPhysicalDevice_T* physicalDevice = nullptr;
    VkDevice_T* logicalDevice = nullptr;
    VkQueue_T* graphicsQueue = nullptr;
    VkQueue_T* presentQueue = nullptr;
    uint32_t graphicsQueueFamilyIndex = 0;
    uint32_t presentQueueFamilyIndex = 0;
    bool anisotropicSamplingSupported = false;
    VkCommandPool_T* vkGenericCommandPool = nullptr;

    VulkanSwapChain* swapChain = nullptr;
    VulkanDevice* deviceFacade = nullptr;

    // Private init helpers
    bool setupValidator();
    void setupInstance();
    void setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT createInfo);
    void setupSurface(GLFWwindow* window);
    void setupDevice(GLFWwindow* window);
    void selectPhysicalDevice();
    void createLogicalDevice();
    void setupSwapchain(GLFWwindow* window);
    void setupGenericCommandPool();
    void makeVkApplicationInfo(VkApplicationInfo& appInfo) const;
    void makeVkDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
    void makeVkInstanceCreateInfo(const VkApplicationInfo& appInfo, VkInstanceCreateInfo& createInfo);
    const char* const* getRequiredExtensions();
};

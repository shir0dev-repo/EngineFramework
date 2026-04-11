#include "../VulkanContext.h"
#include "../VulkanDevice.h"
#include "../VulkanSwapChain.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <map>
#include <set>

// ---------------------------------------------------------------------------
// QueueFamilyIndices — file-local; no longer a public header
// ---------------------------------------------------------------------------
namespace {
    struct QueueFamilyIndex {
        uint32_t index = 0;
        bool exists = false;
    };

    struct QueueFamilyIndices {
        QueueFamilyIndex graphicsFamily;
        QueueFamilyIndex presentFamily;

        bool isComplete() const { return graphicsFamily.exists && presentFamily.exists; }

        static void findQueueFamilies(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface,
                                      QueueFamilyIndices& out) {
            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

            for (uint32_t i = 0; i < count; i++) {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                    if (presentSupport) {
                        out.presentFamily = { i, true };
                    }
                    out.graphicsFamily = { i, true };
                }
                if (out.isComplete()) break;
            }
        }
    };

} // anonymous namespace

// ---------------------------------------------------------------------------
// Debug / validation helpers
// ---------------------------------------------------------------------------
static const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance_T* instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

// ---------------------------------------------------------------------------
// Device helpers
// ---------------------------------------------------------------------------
static const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static bool checkDeviceExtensionSupport(VkPhysicalDevice_T* device) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

    std::set<std::string> required(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& ext : available) {
        required.erase(ext.extensionName);
    }
    return required.empty();
}

static uint32_t rateDeviceSuitability(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface) {
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures   features;
    vkGetPhysicalDeviceProperties(device, &props);
    vkGetPhysicalDeviceFeatures(device, &features);

    QueueFamilyIndices indices{};
    QueueFamilyIndices::findQueueFamilies(device, surface, indices);

    if (!checkDeviceExtensionSupport(device)) return 0;

    if (!features.geometryShader || !indices.isComplete()) return 0;
    if (!VulkanSwapChain::isSwapChainAdequate(device, surface)) return 0;

    uint32_t score = 0;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
    score += props.limits.maxImageDimension2D;
    return score;
}

// ---------------------------------------------------------------------------
// VulkanContext
// ---------------------------------------------------------------------------
VulkanContext* VulkanContext::vulkanContext = nullptr;

VulkanContext* const VulkanContext::getInstance() {
    if (vulkanContext == nullptr) {
        vulkanContext = new VulkanContext();
    }
    return vulkanContext;
}

const VulkanDevice* VulkanContext::getDevice()    const { return deviceFacade; }
VulkanSwapChain*    VulkanContext::getSwapChain() const { return swapChain; }
VkSurfaceKHR_T*     VulkanContext::getSurface()   const { return surface; }

bool VulkanContext::setup(GLFWwindow* window) {
    if (!setupValidator()) return false;
    setupInstance();
    setupSurface(window);
    setupDevice(window);

    deviceFacade = new VulkanDevice(logicalDevice, physicalDevice,
                                    graphicsQueue, presentQueue,
                                    graphicsQueueFamilyIndex, presentQueueFamilyIndex,
                                    anisotropicSamplingSupported);

    setupSwapchain(window);
    setupGenericCommandPool();
    return true;
}

void VulkanContext::teardown() {
    if (vkGenericCommandPool != nullptr) {
        vkDestroyCommandPool(logicalDevice, vkGenericCommandPool, nullptr);
        vkGenericCommandPool = nullptr;
    }
    if (swapChain != nullptr) {
        swapChain->teardown(logicalDevice);
        delete swapChain;
        swapChain = nullptr;
    }
    delete deviceFacade;
    deviceFacade = nullptr;

    if (surface != nullptr) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = nullptr;
    }
    if (logicalDevice != nullptr) {
        vkDestroyDevice(logicalDevice, nullptr);
        logicalDevice = nullptr;
    }
    if (debugMessenger != nullptr) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = nullptr;
    }
    if (instance != nullptr) {
        vkDestroyInstance(instance, nullptr);
        instance = nullptr;
    }
}

VkFormat VulkanContext::querySupportedFormats(VkFormat* candidates, uint32_t candidateCount,
                                              VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for (uint32_t i = 0; i < candidateCount; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);
        if (tiling == VK_IMAGE_TILING_LINEAR  && (props.linearTilingFeatures  & features) == features) return candidates[i];
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return candidates[i];
    }
    throw std::runtime_error("Failed to find supported format!");
}

bool VulkanContext::beginSingleUseCommandBuffer(VkCommandBuffer_T** buffer) const {
    if (vkGenericCommandPool == nullptr) { *buffer = nullptr; return false; }

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkGenericCommandPool;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &(*buffer)) != VK_SUCCESS) return false;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    return vkBeginCommandBuffer(*buffer, &beginInfo) == VK_SUCCESS;
}

void VulkanContext::endSingleUseCommandBuffer(VkCommandBuffer_T* buffer) const {
    vkEndCommandBuffer(buffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(logicalDevice, vkGenericCommandPool, 1, &buffer);
}

// ---------------------------------------------------------------------------
// Private setup methods
// ---------------------------------------------------------------------------
bool VulkanContext::setupValidator() {
    if (!VALIDATION_LAYER_ENABLED) { isValidatorSupported = false; return true; }

    vkEnumerateInstanceLayerProperties(&numAvailableValidationLayers, nullptr);
    std::vector<VkLayerProperties> layers(numAvailableValidationLayers);
    vkEnumerateInstanceLayerProperties(&numAvailableValidationLayers, layers.data());

    isValidatorSupported = true;
    for (const char* layerName : validationLayers) {
        bool found = false;
        for (const auto& lp : layers) {
            if (strcmp(layerName, lp.layerName) == 0) { found = true; numEnabledValidationLayers++; break; }
        }
        if (!found) isValidatorSupported = false;
    }
    return isValidatorSupported;
}

void VulkanContext::setupInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Framework";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (VALIDATION_LAYER_ENABLED) {
        createInfo.enabledLayerCount = numEnabledValidationLayers;
        createInfo.ppEnabledLayerNames = validationLayers.data();
        makeVkDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    createInfo.ppEnabledExtensionNames = getRequiredExtensions();
    createInfo.enabledExtensionCount = numGLFWExtensions;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) throw;

    setupDebugMessenger(debugCreateInfo);
}

void VulkanContext::setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT createInfo) {
    VkResult result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to setup debug messenger! Result: " << result << '\n';
        throw;
    }
}

void VulkanContext::setupSurface(GLFWwindow* window) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void VulkanContext::setupDevice(GLFWwindow* window) {
    selectPhysicalDevice();
    createLogicalDevice();
}

void VulkanContext::selectPhysicalDevice() {
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) throw std::runtime_error("No physical devices supporting Vulkan were found!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto& device : devices) {
        candidates.insert(std::make_pair(rateDeviceSuitability(device, surface), device));
    }

    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
        VkPhysicalDeviceFeatures supported{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &supported);
        anisotropicSamplingSupported = supported.samplerAnisotropy != 0;
    } else {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

void VulkanContext::createLogicalDevice() {
    QueueFamilyIndices queueFamily{};
    QueueFamilyIndices::findQueueFamilies(physicalDevice, surface, queueFamily);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueues = { queueFamily.graphicsFamily.index, queueFamily.presentFamily.index };
    float priority = 1.0f;
    for (uint32_t qf : uniqueQueues) {
        VkDeviceQueueCreateInfo qi{};
        qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = qf;
        qi.queueCount = 1;
        qi.pQueuePriorities = &priority;
        queueCreateInfos.push_back(qi);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    if (anisotropicSamplingSupported) deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (VALIDATION_LAYER_ENABLED) {
        deviceCreateInfo.enabledLayerCount = numEnabledValidationLayers;
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    graphicsQueueFamilyIndex = queueFamily.graphicsFamily.index;
    presentQueueFamilyIndex  = queueFamily.presentFamily.index;
    vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, presentQueueFamilyIndex,  0, &presentQueue);
}

void VulkanContext::setupSwapchain(GLFWwindow* window) {
    swapChain = new VulkanSwapChain();
    swapChain->setup(deviceFacade, surface, window);
}

void VulkanContext::setupGenericCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &vkGenericCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void VulkanContext::makeVkApplicationInfo(VkApplicationInfo& appInfo) const {
    appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Framework";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
}

void VulkanContext::makeVkDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanContext::makeVkInstanceCreateInfo(const VkApplicationInfo& appInfo, VkInstanceCreateInfo& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    if (VALIDATION_LAYER_ENABLED) {
        createInfo.enabledLayerCount = numEnabledValidationLayers;
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    createInfo.enabledExtensionCount = numGLFWExtensions;
    createInfo.ppEnabledExtensionNames = getRequiredExtensions();
}

const char* const* VulkanContext::getRequiredExtensions() {
    static std::vector<const char*> exts;
    if (numGLFWExtensions > 0) return exts.data();

    const char** glfwExts = glfwGetRequiredInstanceExtensions(&numGLFWExtensions);
    exts = std::vector<const char*>(glfwExts, glfwExts + numGLFWExtensions);
    if (VALIDATION_LAYER_ENABLED) { exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); numGLFWExtensions++; }
    numGLFWExtensions = static_cast<uint32_t>(exts.size());
    return exts.data();
}

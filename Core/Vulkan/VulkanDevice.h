#pragma once

typedef unsigned int uint32_t;

struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkQueue_T;

struct VulkanContext;

/// <summary>Read-only facade exposing device handles to the Graphics layer. Owned by VulkanContext.</summary>
struct VulkanDevice {
    VkDevice_T*         getLogicalDevice()            const { return logicalDevice; }
    VkPhysicalDevice_T* getPhysicalDevice()           const { return physicalDevice; }
    VkQueue_T*          getGraphicsQueue()             const { return graphicsQueue; }
    VkQueue_T*          getPresentQueue()              const { return presentQueue; }
    uint32_t            getGraphicsQueueFamilyIndex()  const { return graphicsQueueFamilyIndex; }
    uint32_t            getPresentQueueFamilyIndex()   const { return presentQueueFamilyIndex; }
    bool                supportsAnisotropicSampling()  const { return anisotropicSamplingSupported; }

private:
    friend struct VulkanContext;

    VulkanDevice(VkDevice_T* logicalDevice, VkPhysicalDevice_T* physicalDevice,
                 VkQueue_T* graphicsQueue, VkQueue_T* presentQueue,
                 uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex,
                 bool anisotropicSamplingSupported)
        : logicalDevice(logicalDevice), physicalDevice(physicalDevice),
          graphicsQueue(graphicsQueue), presentQueue(presentQueue),
          graphicsQueueFamilyIndex(graphicsQueueFamilyIndex),
          presentQueueFamilyIndex(presentQueueFamilyIndex),
          anisotropicSamplingSupported(anisotropicSamplingSupported) {}

    VkDevice_T*         logicalDevice             = nullptr;
    VkPhysicalDevice_T* physicalDevice            = nullptr;
    VkQueue_T*          graphicsQueue             = nullptr;
    VkQueue_T*          presentQueue              = nullptr;
    uint32_t            graphicsQueueFamilyIndex  = 0;
    uint32_t            presentQueueFamilyIndex   = 0;
    bool                anisotropicSamplingSupported = false;
};

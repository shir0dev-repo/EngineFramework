#pragma once

typedef unsigned int uint32_t;
typedef uint32_t VkImageUsageFlags;
typedef uint32_t VkMemoryPropertyFlags;
typedef uint32_t VkImageAspectFlags;

typedef enum VkFormat;
typedef enum VkImageTiling;
typedef enum VkImageLayout;

struct VkPhysicalDevice_T;
struct VkImage_T;
struct VkImageView_T;
struct VkDeviceMemory_T;

struct VulkanContext;

uint32_t getMemoryType(VkPhysicalDevice_T* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createVkImage(const VulkanContext* const instance, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage_T*& image, VkDeviceMemory_T*& memory);

void createVkImageView(const VulkanContext* const instance, VkImage_T* image, VkFormat format, VkImageAspectFlags aspectMask, VkImageView_T*& view);

void transitionImageLayout(const VulkanContext* const instance, VkImage_T* image, VkFormat format,
    VkImageLayout previousFormat, VkImageLayout nextFormat);

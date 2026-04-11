#pragma once

typedef enum VkImageLayout;
typedef enum VkFormat;
typedef unsigned long long VkDeviceSize;
typedef unsigned int uint32_t;

struct VkDevice_T;
struct VkImage_T;
struct VkImageView_T;
struct VkDeviceMemory_T;
struct VkSampler_T;
struct VkSamplerCreateInfo;
struct VkImageViewCreateInfo;
struct VulkanContext;

struct GPUTexture {
	static const unsigned int MAX_NAME_SIZE = 64;
	
	char* name = nullptr;

	VkImage_T* image = nullptr;
	VkImageView_T* imageView = nullptr;
	VkDeviceMemory_T* imageMemory = nullptr;
	VkSampler_T* imageSampler = nullptr;

	VkSamplerCreateInfo* samplerInfo = nullptr;
	VkImageViewCreateInfo* imageViewInfo = nullptr;

	VkDeviceSize size = 0l;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channels = 0;

	static GPUTexture* createTexture(const char* filePath, const char* name = "");
	static GPUTexture* createTexture(const uint32_t width, const uint32_t height, const uint32_t channels, 
		const void* data = nullptr, const char* name = "");
	
	static GPUTexture* createTextureLoadImmediate(const VulkanContext* const instance, const char* filePath,
		const char* name = "");
	static GPUTexture* createTextureLoadImmediate(const VulkanContext* const instance, const uint32_t width, const uint32_t height,
		const uint32_t channels, const uint32_t sizeInBytes = 0, const void* data = nullptr, const char* name = "");

	static bool getTexture(const char* name, GPUTexture** outTexture);

	static void transitionLayout(const VulkanContext* const instance, GPUTexture* texture, VkFormat imageFormat,
		VkImageLayout oldLayout, VkImageLayout newLayout);

	static void loadGPU(const VulkanContext* const instance, GPUTexture* texture);
	static void loadGPU(const VulkanContext* const instance, GPUTexture* texture, unsigned char* handle);
	static void loadGPU(const VulkanContext* const instance, const uint32_t sizeInBytes, const void* data, GPUTexture* texture);
	static void setPixels(const VulkanContext* const instance, const uint32_t pixelStride, const uint32_t sizeInBytes, void* pixelData,
		GPUTexture* texture);

	static void cleanup(const VulkanContext* const instance);
private:
	static bool getCPUTextureHandle(GPUTexture* texture, unsigned char** outHandle);

	void createImage(const VulkanContext* const instance);
	void createImageView(const VulkanContext* const instance);
	void createMemory(const VulkanContext* const instance);
	void createSampler(const VulkanContext* const instance);
};
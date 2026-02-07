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
struct VulkanInstance;

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
	static GPUTexture* createTextureLoadImmediate(const VulkanInstance* const instance, const char* filePath,
		const char* name = "");

	static bool getTexture(const char* name, GPUTexture** outTexture);

	static void transitionLayout(const VulkanInstance* const instance, GPUTexture* texture, VkFormat imageFormat,
		VkImageLayout oldLayout, VkImageLayout newLayout);

	static void loadGPU(const VulkanInstance* const instance, GPUTexture* texture);
	static void loadGPU(const VulkanInstance*const instance, GPUTexture* texture, unsigned char* handle);
	static void cleanup(const VulkanInstance* const instance);
private:
	static bool getFilePath(const char* name, char** outFilePath);
	static bool getTextureFromFilePath(const char* filePath, GPUTexture** outTexture);
	static bool getCPUTextureHandle(GPUTexture* texture, unsigned char** outHandle);

	void createImage(const VulkanInstance* const instance);
	void createImageView(const VulkanInstance* const instance);
	void createMemory(const VulkanInstance* const instance);
	void createSampler(const VulkanInstance* const instance);
};
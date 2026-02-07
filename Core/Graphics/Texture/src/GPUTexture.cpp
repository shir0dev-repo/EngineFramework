#include "../GPUTexture.h"
#include "../../BufferUtils.h"
#include "../../../Vulkan/VulkanInstance.h"
#include "../../../Vulkan/VulkanSwapChain.h"
#include "../../../Vulkan/VulkanDevice.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstring>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <iostream>

static std::unordered_map<const char*, const char*> filePathLookup;
static std::unordered_map<const char*, GPUTexture*> textureLookup;
static std::unordered_map<GPUTexture*, stbi_uc*> loadedTextures;

void GPUTexture::cleanup(const VulkanInstance* const instance) {
	for (auto& [texture, cpuHandle] : loadedTextures) {
		stbi_image_free(cpuHandle);
	}

	for (auto& [filePath, texture] : textureLookup) {
		vkDestroySampler(instance->device->logicalDevice, texture->imageSampler, nullptr);
		vkDestroyImageView(instance->device->logicalDevice, texture->imageView, nullptr);
		vkDestroyImage(instance->device->logicalDevice, texture->image, nullptr);
		vkFreeMemory(instance->device->logicalDevice, texture->imageMemory, nullptr);
		delete texture;
	}
}

bool GPUTexture::getFilePath(const char* name, char** outFilePath) {
	uint32_t len = strnlen_s(name, MAX_NAME_SIZE);
	if (len <= 0) {
		return false;
	}

	auto it = filePathLookup.find(name);
	if (it != filePathLookup.end()) {
		uint32_t fpLen = strnlen_s(it->second, MAX_NAME_SIZE);
		strcpy_s(*outFilePath, fpLen, it->second);
		return true;
	}
	else {
		return false;
	}
}

bool GPUTexture::getCPUTextureHandle(GPUTexture* texture, unsigned char** outHandle) {
	auto it = loadedTextures.find(texture);
	if (it != loadedTextures.end()) {
		*outHandle = it->second;
		return true;
	}
	else {
		*outHandle = nullptr;
		return false;
	}
}

GPUTexture* GPUTexture::createTexture(const char* filePath, const char* name) {
	GPUTexture* texture = nullptr;
	char* nameBuffer = nullptr;
	uint32_t nameLen = strnlen_s(name, MAX_NAME_SIZE);

	if (getTextureFromFilePath(filePath, &texture)) {
		return texture;
	}
	else if (strcmp(name, "") == 0) {
		nameBuffer = new char[MAX_NAME_SIZE];
		sprintf_s(nameBuffer, MAX_NAME_SIZE, "Unnamed Texture #%l", (textureLookup.size() - 1));
	}
	else {
		nameBuffer = new char[MAX_NAME_SIZE] {0};
		strcpy_s(nameBuffer, MAX_NAME_SIZE, name);
	}

	texture = new GPUTexture();
	texture->name = nameBuffer;
	int32_t width, height, channels;
	stbi_uc* handle = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
	

	if (!handle) {
		throw std::runtime_error("Failed to load texture asset!");
	}

	if (width >= 0) {
		texture->width = static_cast<uint32_t>(width);
	}
	if (height >= 0) {
		texture->height = static_cast<uint32_t>(height);
	}
	if (channels >= 0) {
		texture->channels = static_cast<uint32_t>(channels);
	}
	texture->size = static_cast<VkDeviceSize>(texture->width) * texture->height * 4;

	
	filePathLookup.emplace(texture->name, filePath);
	textureLookup.emplace(filePath, texture);
	loadedTextures.emplace(texture, handle);
	return texture;
}

GPUTexture* GPUTexture::createTextureLoadImmediate(const VulkanInstance* const instance, const char* filePath, const char* name) {
	
	GPUTexture* texture = createTexture(filePath, name);
	
	stbi_uc* handle = nullptr;
	if (getCPUTextureHandle(texture, &handle)) {
		loadGPU(instance, texture, handle);
	}
	
	return texture;
}

bool GPUTexture::getTexture(const char* name, GPUTexture** outTexture) {
	char* filePath = new char[MAX_NAME_SIZE];
	if (getFilePath(name, &filePath)) {
		bool found = getTextureFromFilePath(const_cast<const char*>(filePath), outTexture);
		delete[] filePath;
		return found;
	}
	else {
		*outTexture = nullptr;
		delete[] filePath;
		return false;
	}
}

bool GPUTexture::getTextureFromFilePath(const char* filePath, GPUTexture** outTexture) {
	auto it = textureLookup.find(filePath);
	if (it != textureLookup.end()) {
		*outTexture = it->second;
		return true;
	}
	else {
		return false;
	}
}

void GPUTexture::loadGPU(const VulkanInstance* const instance, GPUTexture* texture) {
	stbi_uc* handle;
	if (getCPUTextureHandle(texture, &handle)) {
		loadGPU(instance, texture, handle);
	}
}

void GPUTexture::loadGPU(const VulkanInstance* const instance, GPUTexture* texture, unsigned char* handle) {
	if (texture->image != nullptr || texture->imageMemory != nullptr || texture->imageSampler != nullptr) {
		throw std::runtime_error("Texture already loaded to the GPU!");
	}

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingMemory = nullptr;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags memoryUsage = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	BufferUtils::createBuffer(instance->device, texture->size, usage, memoryUsage, &stagingBuffer, &stagingMemory);

	void* data = nullptr;
	vkMapMemory(instance->device->logicalDevice, stagingMemory, 0, texture->size, 0, &data);
	memcpy(data, handle, texture->size);
	vkUnmapMemory(instance->device->logicalDevice, stagingMemory);
	
	loadedTextures.erase(texture);
	stbi_image_free(handle);

	texture->createImage(instance);
	texture->createMemory(instance);
	vkBindImageMemory(instance->device->logicalDevice, texture->image, texture->imageMemory, 0);

	texture->createImageView(instance);
	texture->createSampler(instance);

	GPUTexture::transitionLayout(
		instance,
		texture,
		instance->swapChain->getFormat()->format,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	BufferUtils::copyToImage(instance, stagingBuffer, texture);

	GPUTexture::transitionLayout(
		instance,
		texture,
		instance->swapChain->getFormat()->format,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	vkDestroyBuffer(instance->device->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(instance->device->logicalDevice, stagingMemory, nullptr);
}

void GPUTexture::transitionLayout(const VulkanInstance* const instance, GPUTexture* texture, VkFormat imageFormat,
	VkImageLayout oldLayout, VkImageLayout newLayout) {

	VkCommandBuffer commandBuffer;
	instance->beginSingleUseCommandBuffer(&commandBuffer);
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = texture->image;
	barrier.subresourceRange = texture->imageViewInfo->subresourceRange;
	
	VkPipelineStageFlags srcStage, dstStage;

	switch (oldLayout, newLayout) {
		case (VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			break;
		default:
			throw std::runtime_error("Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer,
		srcStage, dstStage, /** TODO **/
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	instance->endSingleUseCommandBuffer(commandBuffer);
}

void GPUTexture::createImage(const VulkanInstance* const instance) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = this->width;
	imageInfo.extent.height = this->height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	imageInfo.format= instance->swapChain->getFormat()->format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // can be used for things like sparse images

	if (vkCreateImage(instance->device->logicalDevice, &imageInfo, nullptr, &this->image) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image!");
	}
}

void GPUTexture::createImageView(const VulkanInstance* const instance) {
	this->imageViewInfo = new VkImageViewCreateInfo();
	imageViewInfo->sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo->image = this->image;
	imageViewInfo->viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo->format = instance->swapChain->getFormat()->format;
	imageViewInfo->subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo->subresourceRange.baseMipLevel = 0;
	imageViewInfo->subresourceRange.levelCount = 1;
	imageViewInfo->subresourceRange.baseArrayLayer = 0;
	imageViewInfo->subresourceRange.layerCount = 1;
	
	if (vkCreateImageView(instance->device->logicalDevice, this->imageViewInfo, nullptr, &this->imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image view!");
	}
}

void GPUTexture::createMemory(const VulkanInstance* const instance) {
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(instance->device->logicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	uint32_t memType = BufferUtils::getMemoryType(instance->device->physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	allocInfo.memoryTypeIndex = memType;

	if (vkAllocateMemory(instance->device->logicalDevice, &allocInfo, nullptr, &this->imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate image memory!");
	}
}

void GPUTexture::createSampler(const VulkanInstance* const instance) {
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.magFilter = VK_FILTER_LINEAR;
	
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	if (instance->device->deviceSupportsSamplingAnisotropy()) {
		VkPhysicalDeviceProperties deviceProps = {};
		vkGetPhysicalDeviceProperties(instance->device->physicalDevice, &deviceProps);

		sampler.anisotropyEnable = VK_TRUE;
		sampler.maxAnisotropy = deviceProps.limits.maxSamplerAnisotropy;
	}
	else {
		sampler.anisotropyEnable = VK_FALSE;
		sampler.maxAnisotropy = 1.0f;
	}

	sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler.unnormalizedCoordinates = VK_FALSE;
	sampler.compareEnable = VK_FALSE;
	sampler.compareOp = VK_COMPARE_OP_ALWAYS;
	
	/* TODO: implement mipmaps */
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.mipLodBias = 0.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 0.0f;

	if (vkCreateSampler(instance->device->logicalDevice, &sampler, nullptr, &this->imageSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image sampler!");
	}
}
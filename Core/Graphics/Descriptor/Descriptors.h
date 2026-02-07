#pragma once

typedef unsigned int uint32_t;
typedef enum VkDescriptorType;

struct VkDescriptorSet_T;

struct VkDescriptorBufferInfo;
struct VkDescriptorImageInfo;
struct VkWriteDescriptorSet;

struct Descriptors {
	static VkWriteDescriptorSet makeBufferDescriptorWrite(VkDescriptorSet_T* dstSet, uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
	static VkWriteDescriptorSet makeImageSamplerDescriptorWrite(VkDescriptorSet_T* dstSet, uint32_t binding, VkDescriptorImageInfo* imageInfo);
};
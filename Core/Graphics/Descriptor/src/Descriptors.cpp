#include "../Descriptors.h"

#include <vulkan/vulkan.h>

VkWriteDescriptorSet Descriptors::makeBufferDescriptorWrite(VkDescriptorSet_T* dstSet, uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
	VkWriteDescriptorSet out = {};
	out.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	out.dstSet = dstSet;
	out.dstBinding = binding;
	out.dstArrayElement = 0;
	out.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	out.descriptorCount = 1;
	out.pBufferInfo = bufferInfo;
	return out;
}

VkWriteDescriptorSet Descriptors::makeImageSamplerDescriptorWrite(VkDescriptorSet_T* dstSet, uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
	VkWriteDescriptorSet out = {};
	out.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	out.dstSet = dstSet;
	out.dstBinding = binding;
	out.dstArrayElement = 0;
	out.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	out.descriptorCount = 1;
	out.pImageInfo = imageInfo;
	return out;
}
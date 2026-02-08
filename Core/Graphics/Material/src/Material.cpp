#include "../Material.h"
#include "../../../Vulkan/VulkanInstance.h"
#include "../../../Vulkan/VulkanDevice.h"
#include "../../Pipeline/GraphicsPipeline.h"
#include "../../Pipeline/PipelineMaterialLayout.h"
#include "../../Renderer/Renderer.h"
#include "../../Texture/GPUTexture.h"
#include "../../Shader/GPUBuffer.h"
#include "../../Shader/Reflection.h"

#include <cstring>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

static std::unordered_map<const char*, Material*> materialLookup;

void Material::cleanup(const VulkanInstance* const instance) {
	for (auto& [name, material] : materialLookup) {
		if (material->pBuffers != nullptr) {
			for (uint32_t i = 0; i < material->numBufferHandles; i++) {
				material->pBuffers[i].handle->dispose(instance->device->logicalDevice);
				delete material->pBuffers[i].handle;
			}
			delete[] material->pBuffers;
		}

		delete material;
	}
}

Material* const Material::create(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const char* name) {
	Material* material = nullptr;
	if (find(name, &material)) {
		return material;
	}

	material = new Material();
	material->pipeline = pipeline;
	
	const PipelineMaterialLayout* layout = pipeline->getMaterialLayout();
	std::vector<MaterialBinding> textureBindings = {};
	std::vector<MaterialBinding> bufferBindings = {};
	std::vector<MaterialBinding> allBindings = {};

	for (uint32_t i = 0; i < layout->numBindings; i++) {
		MaterialBinding binding = layout->pBindings[i];
		if (binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			bufferBindings.push_back(binding);
		}
		else if (binding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
			textureBindings.push_back(binding);
		}

		allBindings.push_back(binding);
	}

	const PipelineSummary* summary = pipeline->getSummary();
	material->vkDescriptorSets = new VkDescriptorSet_T* [pipeline->getDescriptorCopyCount()];
	pipeline->generateMaterialDescriptorSets(instance, material->vkDescriptorSets);

	material->createTextureHandles(instance, pipeline, summary, textureBindings.data(), textureBindings.size());
	material->createBufferHandles(instance, pipeline, summary, bufferBindings.data(), bufferBindings.size());

	for (uint32_t copyIndex = 0; copyIndex < pipeline->getDescriptorCopyCount(); copyIndex++) {
		std::vector<VkDescriptorImageInfo> imageInfos = {};
		std::vector<VkDescriptorBufferInfo> bufferInfos = {};
		std::vector<VkWriteDescriptorSet> writes = {};
		uint32_t currBuffer = 0, currTexture = 0;
		for (uint32_t i = 0; i < textureBindings.size(); i++) {
			MaterialBinding binding = textureBindings.at(i);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = material->pTextures[i].defaultHandle->imageView;
			imageInfo.sampler = material->pTextures[i].defaultHandle->imageSampler;
			imageInfos.push_back(imageInfo);

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstBinding = binding.binding;
			write.dstSet = material->vkDescriptorSets[copyIndex];
			write.pImageInfo = &imageInfo;
			writes.push_back(write);
		}

		for (uint32_t i = 0; i < bufferBindings.size(); i++) {
			MaterialBinding binding = bufferBindings.at(i);

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.offset = 0;
			bufferInfo.range = material->pBuffers[i].handle->bufferSize;
			bufferInfo.buffer = material->pBuffers[i].handle->vkBuffer;

			bufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstBinding = binding.binding;
			write.dstSet = material->vkDescriptorSets[copyIndex];
			write.pBufferInfo = &bufferInfo;

			writes.push_back(write);
		}

		vkUpdateDescriptorSets(instance->device->logicalDevice, writes.size(), writes.data(), 0, nullptr);
	}
	materialLookup.emplace(name, material);
	pipeline->registerMaterial(material);
	
	return material;
}



bool Material::find(const char* name, Material** outMaterial) {
	auto it = materialLookup.find(name);
	if (it != materialLookup.end()) {
		*outMaterial = it->second;
		return true;
	}
	else {
		*outMaterial = nullptr;
		return false;
	}
}

void Material::createTextureHandles(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const PipelineSummary* const summary,
	MaterialBinding* bindings, uint32_t bindingCount) {
	
	this->numTextureHandles = bindingCount;
	if (bindingCount <= 0) {
		this->pTextures = nullptr;
		return;
	}

	this->pTextures = new TextureHandleEntry[bindingCount];
	for (uint32_t i = 0; i < bindingCount; i++) {
		TextureHandleEntry* entry = &this->pTextures[i];
		
		MaterialBinding bindingInfo = bindings[i];
		entry->binding = bindingInfo.binding;

		GPUTexture::getTexture("white", &entry->defaultHandle);
		entry->customHandle = nullptr;
	}

	/*for (uint32_t copyIndex = 0; copyIndex < pipeline->getDescriptorCopyCount(); copyIndex++) {
		std::vector<VkDescriptorImageInfo> imageInfos = {};
		std::vector<VkWriteDescriptorSet> writes = {};

		for (uint32_t i = 0; i < this->numTextureHandles; i++) {
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = pTextures[i].defaultHandle->imageView;
			imageInfo.sampler = pTextures[i].defaultHandle->imageSampler;
			imageInfos.push_back(imageInfo);

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstBinding = pTextures[i].binding;
			write.dstSet = vkDescriptorSets[copyIndex];
			write.pImageInfo = &imageInfo;
			writes.push_back(write);
		}

		vkUpdateDescriptorSets(instance->device->logicalDevice, writes.size(), writes.data(), 0, nullptr);
	}*/
}

void Material::createBufferHandles(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const PipelineSummary* const summary,
	MaterialBinding* bindings, uint32_t bindingCount) {

	this->numBufferHandles = bindingCount;
	if (bindingCount <= 0) {
		this->pBuffers = nullptr;
		return;
	}

	this->pBuffers = new BufferHandleEntry[numBufferHandles] { {} };
	for (uint32_t i = 0; i < bindingCount; i++) {
		BufferHandleEntry* entry = &this->pBuffers[i];
		MaterialBinding bindingInfo = bindings[i];

		DescriptorBindingInfo descInfo = summary->materialDescriptors.pDescriptorBindingInfos[bindingInfo.binding];
		entry->handle = GPUBuffer::create(instance, descInfo.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}
	/*for (uint32_t copyIndex = 0; copyIndex < pipeline->getDescriptorCopyCount(); copyIndex++) {
		std::vector<VkDescriptorBufferInfo> bufferInfos = {};
		std::vector<VkWriteDescriptorSet> writes = {};

		for (uint32_t i = 0; i < this->numBufferHandles; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.offset = 0;
			bufferInfo.range = pBuffers->handle->bufferSize;
			bufferInfo.buffer = pBuffers->handle->vkBuffer;
			
			bufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstBinding = pBuffers[i].binding;
			write.dstSet = vkDescriptorSets[copyIndex];
			write.pBufferInfo = &bufferInfo;

			writes.push_back(write);
		}

		vkUpdateDescriptorSets(instance->device->logicalDevice, writes.size(), writes.data(), 0, nullptr);
	}*/
}

void Material::bind(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 2, 1, &this->vkDescriptorSets[currentFrame], 0, nullptr);
}

void Material::unbind(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const {
	
}
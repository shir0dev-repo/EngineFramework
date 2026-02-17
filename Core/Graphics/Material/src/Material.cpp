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
		if (material->pInstanceBuffers != nullptr) {
			for (uint32_t i = 0; i < material->numInstanceBufferHandles; i++) {
				material->pInstanceBuffers[i].handle->dispose(instance->device->logicalDevice);
				delete material->pInstanceBuffers[i].handle;
			}
			delete[] material->pInstanceBuffers;
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
	std::vector<MaterialBinding> instanceBindings = {};
	std::vector<MaterialBinding> allBindings = {};

	for (uint32_t i = 0; i < layout->numBindings; i++) {
		MaterialBinding binding = layout->pBindings[i];
		if (binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			if (binding.set == Renderer::MATERIAL_DESCRIPTOR_SET) {
				bufferBindings.push_back(binding);
			}
			else if (binding.set == Renderer::INSTANCE_DESCRIPTOR_SET) {
				instanceBindings.push_back(binding);
			}
		}
		else if (binding.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
			textureBindings.push_back(binding);
		}

		allBindings.push_back(binding);
	}

	const PipelineSummary* summary = pipeline->getSummary();
	material->vkDescriptorSets = new VkDescriptorSet_T* [pipeline->getDescriptorCopyCount()];
	material->vkInstanceDescriptorSets = new VkDescriptorSet_T* [pipeline->getDescriptorCopyCount()];
	pipeline->generateMaterialDescriptorSets(instance, material->vkDescriptorSets);
	pipeline->generateInstanceDescriptorSets(instance, material->vkInstanceDescriptorSets);

	material->createTextureHandles(instance, pipeline, summary, textureBindings.data(), textureBindings.size());
	material->createBufferHandles(instance, pipeline, summary, bufferBindings.data(), bufferBindings.size());
	material->createInstanceBufferHandles(instance, pipeline, summary, instanceBindings.data(), instanceBindings.size());

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
		for (uint32_t i = 0; i < instanceBindings.size(); i++) {
			MaterialBinding binding = instanceBindings.at(i);

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.offset = 0;
			bufferInfo.range = material->pInstanceBuffers[i].handle->bufferSize;
			bufferInfo.buffer = material->pInstanceBuffers[i].handle->vkBuffer;

			bufferInfos.push_back(bufferInfo);

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstBinding = binding.binding;
			write.dstSet = material->vkInstanceDescriptorSets[copyIndex];
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

		DescriptorBindingInfo descInfo = summary->materialDescriptors.pDescriptorBindingInfos[i];
		this->pBuffers[i].binding = bindingInfo.binding;
		this->pBuffers[i].handle = GPUBuffer::create(instance, descInfo.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}
}

void Material::createInstanceBufferHandles(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const PipelineSummary* const summary,
	MaterialBinding* bindings, uint32_t bindingCount) {

	this->numInstanceBufferHandles = bindingCount;
	if (bindingCount <= 0) {
		this->pInstanceBuffers = nullptr;
		return;
	}

	this->pInstanceBuffers = new BufferHandleEntry[numInstanceBufferHandles]{ {} };
	for (uint32_t i = 0; i < bindingCount; i++) {
		BufferHandleEntry* entry = &this->pInstanceBuffers[i];
		MaterialBinding bindingInfo = bindings[i];

		DescriptorBindingInfo descInfo = summary->instanceDescriptors.pDescriptorBindingInfos[i];
		this->pInstanceBuffers[i].binding = bindingInfo.binding;
		this->pInstanceBuffers[i].handle = GPUBuffer::create(instance, descInfo.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	}
}

void Material::bind(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 2, 1, &this->vkDescriptorSets[currentFrame], 0, nullptr);
	bindInstance(commandBuffer, currentFrame);
}
void Material::bindInstance(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 3, 1, &this->vkInstanceDescriptorSets[currentFrame], 0, nullptr);
}

void Material::unbind(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const {
	
}

void Material::setTexture(const VulkanInstance* const instance, GPUTexture* texture, uint32_t binding) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->imageView;
	imageInfo.sampler = texture->imageSampler;

	for (uint32_t i = 0; i < 3; i++) {
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.dstSet = vkDescriptorSets[i];
		write.dstBinding = binding;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(instance->device->logicalDevice, 1, &write, 0, nullptr);
	}

	pTextures[binding].customHandle = texture;
}

void Material::setFloat(const VulkanInstance* const instance, uint32_t binding, float value) const {
	for (uint32_t i = 0; i < numBufferHandles; i++) {
		GPUBuffer* handle = this->pBuffers[i].handle;
		void* data = &value;
		handle->bufferData(instance, data, sizeof(float), 0);
	}
}

void Material::setBuffer(const VulkanInstance* const instance, uint32_t set, uint32_t binding, const void* data, uint32_t size) const {
	uint32_t count = set == 2 ? numBufferHandles : numInstanceBufferHandles;
	BufferHandleEntry* targetBuffers = set == 2 ? pBuffers : pInstanceBuffers;
	for (uint32_t i = 0; i < count; i++) {
		BufferHandleEntry* handle = &targetBuffers[i];
		if (handle->binding != binding) {
			continue;
		}

		handle->handle->bufferData(instance, data, size);
	}
}
#include "Core/Graphics/Buffer/MeshBuffer.h"

#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"

#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Buffer/GPUBuffer.h"

#include <vulkan/vulkan.h>

void MeshBuffer::setup(const VulkanContext* const instance, Renderer* renderer, Mesh* const meshRef) {
	this->mesh = meshRef;

	VkBufferUsageFlags vertexFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	this->vertexBuffer = GPUBuffer::create(instance, meshRef->vertexCount * sizeof(Vertex), vertexFlags, meshRef->vertexData);

	VkBufferUsageFlags indexFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	this->indexBuffer = GPUBuffer::create(instance, meshRef->indexCount * sizeof(uint32_t), indexFlags, meshRef->indexData);
}

void MeshBuffer::teardown(const VulkanContext* const instance) {
	if (vertexBuffer != nullptr) {
		vertexBuffer->dispose(instance->getDevice()->getLogicalDevice());
		delete vertexBuffer;
		vertexBuffer = nullptr;
	}
	if (indexBuffer != nullptr) {
		indexBuffer->dispose(instance->getDevice()->getLogicalDevice());
		delete indexBuffer;
		indexBuffer = nullptr;
	}
}
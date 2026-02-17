#include "../MeshRenderer.h"
#include "../Renderer.h"
#include "../../Shader/Vertex.h"
#include "../../Mesh/Mesh.h"
#include "../../Shader/GPUBuffer.h"
#include "../../Pipeline/GraphicsPipeline.h"
#include "../../../Vulkan/VulkanInstance.h"
#include "Runtime/Scene/Entity.h"

#include <shml/matrix4f.hpp>
#include <vulkan/vulkan.h>

void MeshRenderer::setup(const VulkanInstance* const instance, Entity* const entity, Mesh* meshRef, Material* materialRef, Renderer* renderer) {
	this->mesh = meshRef;
	this->material = materialRef;
	this->entity = entity;

	VkBufferUsageFlags vertexFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	this->vertexBuffer = GPUBuffer::create(instance, meshRef->vertexCount * sizeof(Vertex), vertexFlags, meshRef->vertexData);
	
	VkBufferUsageFlags indexFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	this->indexBuffer = GPUBuffer::create(instance, meshRef->indexCount * sizeof(uint32_t), indexFlags, meshRef->indexData);
	
	VkBufferUsageFlags transformFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (entity == nullptr) {
		this->transformBuffer = GPUBuffer::create(instance, sizeof(shml::matrix4f), transformFlags);
	}
	else {
		this->transformBuffer = GPUBuffer::create(instance, sizeof(shml::matrix4f), transformFlags, entity->getTransform().getPointer());
	}
}

void MeshRenderer::teardown(VkDevice_T* logicalDevice) {
	if (vertexBuffer != nullptr) {
		vertexBuffer->dispose(logicalDevice);
		delete vertexBuffer;
		vertexBuffer = nullptr;
	}
	if (indexBuffer != nullptr) {
		indexBuffer->dispose(logicalDevice);
		delete indexBuffer;
		indexBuffer = nullptr;
	}
	if (transformBuffer != nullptr) {
		transformBuffer->dispose(logicalDevice);
		delete transformBuffer;
		transformBuffer = nullptr;
	}
	mesh = nullptr;
}

void MeshRenderer::draw(GraphicsPipeline* pipeline) {
	pipeline->addRenderCommand(this);
}
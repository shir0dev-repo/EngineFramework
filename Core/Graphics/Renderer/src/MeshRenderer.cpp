#include "../MeshRenderer.h"
#include "../../../Vulkan/VulkanDevice.h"
#include "../../Shader/Vertex.h"
#include "../../Mesh/Mesh.h"
#include "../../Shader/GPUBuffer.h"
#include "../../GraphicsPipeline.h"

#include <vulkan/vulkan.h>

void MeshRenderer::setup(const VulkanDevice* const device, Mesh* meshRef) {
	this->mesh = meshRef;
	
	VkBufferUsageFlags vertexFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	this->vertexBuffer = GPUBuffer::create(device, meshRef->vertexCount * sizeof(Vertex), vertexFlags, meshRef->vertexData);
	
	VkBufferUsageFlags indexFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	this->indexBuffer = GPUBuffer::create(device, meshRef->indexCount * sizeof(uint32_t), indexFlags, meshRef->indexData);
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

	mesh = nullptr;
}

void MeshRenderer::draw(GraphicsPipeline* pipeline) {
	pipeline->addRenderCommand(vertexBuffer, 1, 3, 0);
}
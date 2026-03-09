#include "Core/Entity/Component/MeshRenderer.h"

#include "Core/Vulkan/VulkanInstance.h"
#include "Core/Vulkan/VulkanDevice.h"

#include "Core/Graphics/Pipeline/GraphicsPipeline.h"
#include "Core/Graphics/Renderer/Renderer.h"

#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Buffer/GPUBuffer.h"
#include "Core/Graphics/Buffer/MeshBuffer.h"

#include <shml/matrix4f.hpp>
#include <vulkan/vulkan.h>

MeshRenderer::MeshRenderer(const VulkanInstance* const instance, Renderer* renderer, Entity* const entity, Mesh* const meshRef, Material* const materialRef)
: IEntityComponent(entity) {
	this->material = materialRef;
	
	this->meshBuffer = new MeshBuffer();
	meshBuffer->setup(instance, renderer, meshRef);
	if (entity) {
		this->transformBuffer = GPUBuffer::create(instance, sizeof(shml::matrix4f), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, entity->getTransform().getPointer());
	}
	else {
		transformBuffer = nullptr;
	}
}

void MeshRenderer::teardown(const VulkanInstance* const instance) {
	if (meshBuffer != nullptr) {
		meshBuffer->teardown(instance);
	}
	if (transformBuffer != nullptr) {
		transformBuffer->dispose(instance->device->logicalDevice);
		delete transformBuffer;
		transformBuffer = nullptr;
	}
}

GPUBuffer* const MeshRenderer::getVertexBuffer() const {
	return meshBuffer->vertexBuffer;
}
GPUBuffer* const MeshRenderer::getIndexBuffer() const {
	return meshBuffer->indexBuffer;
}
GPUBuffer* const MeshRenderer::getTransformBuffer() const {
	return transformBuffer;
}

uint32_t MeshRenderer::getVertexCount() const {
	return meshBuffer->mesh->vertexCount;
}
uint32_t MeshRenderer::getIndexCount() const {
	return meshBuffer->mesh->indexCount;
}

void MeshRenderer::draw(GraphicsPipeline* pipeline) {
	pipeline->addRenderCommand(this);
}
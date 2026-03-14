#include "Core/Entity/Component/IRenderable.h"

#include "Core/Entity/Entity.h"
#include "Core/Vulkan/VulkanInstance.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Graphics/Buffer/GPUBuffer.h"
#include "Core/Graphics/Buffer/MeshBuffer.h"

#include <vulkan/vulkan.h>

void IRenderable::setup(const VulkanInstance* const instance, Renderer* const renderer, Entity* const entityRef, Mesh* const meshRef, Material* const materialRef) {
	this->meshBuffer = new MeshBuffer();
	meshBuffer->setup(instance, renderer, meshRef);
	this->material = materialRef;

	if (entityRef) {
		transformBuffer = GPUBuffer::create(instance, sizeof(shml::matrix4f), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, entityRef->getTransform().getPointer());
	}
	else {
		transformBuffer = nullptr;
	}
}

GPUBuffer* const IRenderable::getVertexBuffer() const {
	if (!meshBuffer) {
		return nullptr;
	}
	
	return meshBuffer->vertexBuffer;
}

uint32_t IRenderable::getVertexCount() const {
	if (!meshBuffer) {
		return 0;
	}

	return meshBuffer->mesh->vertexCount;
}

GPUBuffer* const IRenderable::getIndexBuffer() const {
	if (!meshBuffer) {
		return nullptr;
	}

	return meshBuffer->indexBuffer;
}

uint32_t IRenderable::getIndexCount() const {
	if (!meshBuffer) {
		return 0;
	}

	return meshBuffer->mesh->indexCount;
}

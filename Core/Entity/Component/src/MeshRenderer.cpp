#include "Core/Entity/Component/MeshRenderer.h"

#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanDevice.h"
#include "Core/Graphics/Pipeline/GraphicsPipeline.h"
#include "Core/Graphics/Renderer/Renderer.h"

#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Buffer/GPUBuffer.h"
#include "Core/Graphics/Buffer/MeshBuffer.h"
#include "Core/Graphics/Material/Material.h"

#include <shml/matrix4f.hpp>
#include <vulkan/vulkan.h>

MeshRenderer::MeshRenderer() : IEntityComponent(nullptr) {
	this->material = nullptr;
}

MeshRenderer::MeshRenderer(const VulkanContext* const instance, Renderer* renderer, Entity* const entity, Mesh* const meshRef, Material* const materialRef)
: IEntityComponent(entity) {
	this->material = materialRef;
	
	IRenderable::setup(instance, renderer, entity, meshRef, materialRef);
}

void MeshRenderer::teardown(const VulkanContext* const instance) {
	if (meshBuffer != nullptr) {
		meshBuffer->teardown(instance);
	}
	if (transformBuffer != nullptr) {
		transformBuffer->dispose(instance->getDevice()->getLogicalDevice());
		delete transformBuffer;
		transformBuffer = nullptr;
	}
}

void MeshRenderer::updateTransform(const shml::matrix4f& transform) {
	if (transformBuffer) {
		transformBuffer->bufferData(VulkanContext::getInstance(),
		                            transform.getPointer(),
		                            sizeof(shml::matrix4f));
	}
	if (material) {
		material->setBuffer(VulkanContext::getInstance(), 3, 0,
		                    transform.getPointer(), sizeof(shml::matrix4f));
	}
}

void MeshRenderer::draw() {
	if (!material) {
		return;
	}

	material->pipeline->addRenderCommand(this);
}
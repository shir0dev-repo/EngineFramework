#include "Core/Entity/Component/MeshRenderer.h"

#include "Core/Vulkan/VulkanInstance.h"
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

MeshRenderer::MeshRenderer(const VulkanInstance* const instance, Renderer* renderer, Entity* const entity, Mesh* const meshRef, Material* const materialRef)
: IEntityComponent(entity) {
	this->material = materialRef;
	
	IRenderable::setup(instance, renderer, entity, meshRef, materialRef);
}

void MeshRenderer::teardown(const VulkanInstance* const instance) {
	if (meshBuffer != nullptr) {
		meshBuffer->teardown(instance);
	}
	if (transformBuffer != nullptr) {
		transformBuffer->dispose(instance->logicalDevice);
		delete transformBuffer;
		transformBuffer = nullptr;
	}
}

void MeshRenderer::draw() {
	if (!material) {
		return;
	}

	material->pipeline->addRenderCommand(this);
}
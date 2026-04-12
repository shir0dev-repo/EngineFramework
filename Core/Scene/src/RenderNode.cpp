#include "Core/Scene/RenderNode.h"

#include "Core/Scene/World.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Entity/Component/MeshRenderer.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"

RenderNode::RenderNode(Renderer* renderer, Mesh* const meshRef, Material* const materialRef) {
	auto instance = VulkanContext::getInstance();
	this->meshRenderer = new MeshRenderer(instance, renderer, this, meshRef, materialRef);
}

RenderNode::~RenderNode() {
	if (meshRenderer != nullptr) {
		meshRenderer->teardown(VulkanContext::getInstance());
	}
}

void RenderNode::update(World* world, const float dt) {
	if (meshRenderer) {
		meshRenderer->updateTransform(this->getTransform());
		meshRenderer->draw();
	}
}

void RenderNode::draw(World* world) {
	
}
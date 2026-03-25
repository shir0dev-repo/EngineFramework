#include "Core/Scene/RenderNode.h"

#include "Core/Scene/World.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Entity/Component/MeshRenderer.h"
#include "Core/Vulkan/VulkanInstance.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Material/Material.h"

RenderNode::RenderNode(SceneNode* parent, Renderer* renderer, Mesh* const meshRef, Material* const materialRef) {
	if (parent) {
		this->parent = parent;
	}
	else {
		this->parent = World::getWorld()->getRootNode();
	}

	auto instance = VulkanInstance::getInstance();
	this->meshRenderer = new MeshRenderer(instance, renderer, this, meshRef, materialRef);
}

RenderNode::~RenderNode() {
	if (meshRenderer != nullptr) {
		meshRenderer->teardown(VulkanInstance::getInstance());
	}
}

void RenderNode::update(World* world, const float dt) {
	if (meshRenderer) {
		meshRenderer->draw();
	}
}
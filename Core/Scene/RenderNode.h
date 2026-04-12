#pragma once

#include "Core/Scene/SceneNode.h"

struct RenderNode : public SceneNode {
	RenderNode(struct Renderer* renderer, struct Mesh* const meshRef, struct Material* const materialRef);
	
	virtual ~RenderNode() override;
	virtual void update(class World* world, float dt) override;
	virtual void draw(class World* world) override;

	struct MeshRenderer* meshRenderer = nullptr;
};
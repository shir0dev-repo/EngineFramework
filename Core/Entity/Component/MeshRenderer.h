#pragma once

#include "Core/Entity/Component/IEntityComponent.h"
#include "Core/Entity/Component/IRenderable.h"

typedef unsigned int uint32_t;

struct VkDevice_T;

struct Mesh;
struct GPUBuffer;
struct GraphicsPipeline;
struct VulkanInstance;
struct Renderer;
struct Material;
struct Entity;
struct MeshBuffer;

struct MeshRenderer : public IEntityComponent, public IRenderable {
	
	MeshRenderer();
	MeshRenderer(const VulkanInstance* const instance, Renderer* renderer, Entity* const entity, Mesh* const meshRef, Material* const materialRef);
	void teardown(const VulkanInstance* const instance);

	virtual void draw() override;
};
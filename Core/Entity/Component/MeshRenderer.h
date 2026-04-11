#pragma once

#include "Core/Entity/Component/IEntityComponent.h"
#include "Core/Entity/Component/IRenderable.h"
#include <shml/matrix4f.hpp>
#include <cstdint>

struct VkDevice_T;

struct Mesh;
struct GPUBuffer;
struct GraphicsPipeline;
struct VulkanContext;
struct Renderer;
struct Material;
struct Entity;
struct MeshBuffer;

struct MeshRenderer : public IEntityComponent, public IRenderable {
	
	MeshRenderer();
	MeshRenderer(const VulkanContext* const instance, Renderer* renderer, Entity* const entity, Mesh* const meshRef, Material* const materialRef);
	void teardown(const VulkanContext* const instance);

	void updateTransform(const shml::matrix4f& transform);
	virtual void draw() override;
};
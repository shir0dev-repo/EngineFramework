#pragma once

#include "Core/Entity/Component/IEntityComponent.h"

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

struct MeshRenderer : public IEntityComponent {
	MeshBuffer* meshBuffer = nullptr;
	Material* material = nullptr;

	GPUBuffer* const getVertexBuffer() const;
	uint32_t getVertexCount() const;
	GPUBuffer* const getIndexBuffer() const;
	uint32_t getIndexCount() const;
	GPUBuffer* const getTransformBuffer() const;

	MeshRenderer(const VulkanInstance* const instance, Renderer* renderer, Entity* const entity, Mesh* const meshRef, Material* const materialRef);
	void teardown(const VulkanInstance* const instance);

	void draw(GraphicsPipeline* pipeline);

private:
	GPUBuffer* transformBuffer;
};
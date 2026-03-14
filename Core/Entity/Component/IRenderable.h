#pragma once

struct VulkanInstance;
struct Renderer;
struct Entity;
struct Mesh;
struct MeshBuffer;
struct GPUBuffer;
struct Material;

struct IRenderable {
	MeshBuffer* meshBuffer = nullptr;
	GPUBuffer* transformBuffer = nullptr;
	Material* material = nullptr;

	virtual void setup(const VulkanInstance* const instance, Renderer* const renderer, Entity* const entityRef, Mesh* const meshRef, Material* const materialRef);

	GPUBuffer* const getVertexBuffer() const;
	unsigned int getVertexCount() const;

	GPUBuffer* const getIndexBuffer() const;
	unsigned int getIndexCount() const;

	virtual void draw() = 0;
};
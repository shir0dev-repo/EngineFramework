#pragma once

struct VulkanContext;
struct Renderer;
struct Entity;
struct Mesh;
struct MeshBuffer;
struct GPUBuffer;
struct Material;

struct IRenderable {
	virtual void setup(const VulkanContext* const instance, Renderer* const renderer, Entity* const entityRef, Mesh* const meshRef, Material* const materialRef);

	GPUBuffer* const getVertexBuffer() const;
	unsigned int getVertexCount() const;

	GPUBuffer* const getIndexBuffer() const;
	unsigned int getIndexCount() const;

	virtual void draw() = 0;

	MeshBuffer* const getMeshBuffer() const { return meshBuffer; }
	GPUBuffer* const getTransformBuffer() const { return transformBuffer; }
	Material* const getMaterial() const { return material; }

protected:
	MeshBuffer* meshBuffer      = nullptr;
	GPUBuffer*  transformBuffer = nullptr;
	Material*   material        = nullptr;
};
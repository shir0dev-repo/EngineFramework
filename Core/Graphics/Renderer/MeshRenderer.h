#pragma once

struct VkDevice_T;

struct Mesh;
struct GPUBuffer;
struct GraphicsPipeline;
struct VulkanInstance;
struct Renderer;
struct Material;
struct Entity;

struct MeshRenderer {
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	Entity* entity = nullptr;

	GPUBuffer* const getVertexBuffer() const { return vertexBuffer; }
	GPUBuffer* const getIndexBuffer() const { return indexBuffer; }
	GPUBuffer* const getTransformBuffer() const { return transformBuffer; }

	void setup(const VulkanInstance* const device, Entity* const entity, Mesh* const meshRef, Material* const materialRef, Renderer* renderer);
	void teardown(VkDevice_T* logicalDevice);
	void draw(GraphicsPipeline* pipeline);

private:
	GPUBuffer* vertexBuffer;
	GPUBuffer* indexBuffer;
	GPUBuffer* transformBuffer;
};
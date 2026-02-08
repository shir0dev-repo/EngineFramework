#pragma once

struct VkDevice_T;

struct Mesh;
struct GPUBuffer;
struct GraphicsPipeline;
struct VulkanInstance;
struct Renderer;
struct Material;

struct MeshRenderer {
	Mesh* mesh = nullptr;
	Material* material = nullptr;

	GPUBuffer* const getVertexBuffer() const { return vertexBuffer; }
	GPUBuffer* const getIndexBuffer() const { return indexBuffer; }

	void setup(const VulkanInstance* const device, Mesh* const meshRef, Material* const materialRef, Renderer* renderer);
	void teardown(VkDevice_T* logicalDevice);
	void draw(GraphicsPipeline* pipeline);

private:
	GPUBuffer* vertexBuffer;
	GPUBuffer* indexBuffer;
};
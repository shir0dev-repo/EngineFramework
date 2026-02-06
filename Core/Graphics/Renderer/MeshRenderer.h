#pragma once

struct VkDevice_T;

struct Mesh;
struct GPUBuffer;
struct GraphicsPipeline;
struct VulkanDevice;
struct Renderer;

struct MeshRenderer {
	Mesh* mesh;

	GPUBuffer* const getVertexBuffer() const { return vertexBuffer; }
	GPUBuffer* const getIndexBuffer() const { return indexBuffer; }

	void setup(const VulkanDevice* const device, Mesh* const meshRef, Renderer* renderer);
	void teardown(VkDevice_T* logicalDevice);
	void draw(GraphicsPipeline* pipeline);

private:
	GPUBuffer* vertexBuffer;
	GPUBuffer* indexBuffer;
};
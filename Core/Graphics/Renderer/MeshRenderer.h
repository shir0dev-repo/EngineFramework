#pragma once

struct VkDevice_T;

struct Mesh;
struct GPUBuffer;
struct GraphicsPipeline;
struct VulkanDevice;

struct MeshRenderer {
	Mesh* mesh;

	void setup(const VulkanDevice* const device, Mesh* const meshRef);
	void teardown(VkDevice_T* logicalDevice);
	void draw(GraphicsPipeline* pipeline);

private:
	GPUBuffer* vertexBuffer;
	GPUBuffer* indexBuffer;
};
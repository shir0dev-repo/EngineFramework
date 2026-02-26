#pragma once

struct VulkanInstance;
struct Renderer;
struct GPUBuffer;
struct Mesh;

struct VkBuffer_T;

struct MeshBuffer {
	Mesh* mesh = nullptr;
	GPUBuffer* vertexBuffer = nullptr;
	GPUBuffer* indexBuffer = nullptr;

	void setup(const VulkanInstance* const instance, Renderer* renderer, Mesh* const meshRef);
	void teardown(const VulkanInstance* const instance);
};
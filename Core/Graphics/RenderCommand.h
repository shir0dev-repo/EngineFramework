#pragma once

typedef unsigned int uint32_t;

struct VkCommandBuffer_T;

struct GPUBuffer;
struct Material;
struct RenderCommand {
	GPUBuffer* vertexBuffer = nullptr;
	GPUBuffer* indexBuffer = nullptr;
	GPUBuffer* transformBuffer = nullptr;

	Material* material = nullptr;
	uint32_t bufferCount;
	uint32_t vertexCount;
	uint32_t indexCount;

	static RenderCommand* create(Material* material, GPUBuffer* vertexBuffer, GPUBuffer* indexBuffer, uint32_t vertexCount, uint32_t indexCount,
		GPUBuffer* transformBuffer = nullptr);
	void execute(VkCommandBuffer_T* commandBuffer);
};
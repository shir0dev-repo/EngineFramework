#pragma once

typedef unsigned int uint32_t;

struct VkCommandBuffer_T;

struct GPUBuffer;

struct RenderCommand {
	GPUBuffer* targetBuffers = nullptr;
	uint32_t bufferCount;
	uint32_t vertexCount;
	uint32_t indexCount;

	static RenderCommand* create(GPUBuffer* buffers, uint32_t bufferCount, uint32_t vertexCount, uint32_t indexCount);
	void execute(VkCommandBuffer_T* commandBuffer);
};
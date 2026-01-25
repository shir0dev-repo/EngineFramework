#include "../RenderCommand.h"
#include "../Shader/GPUBuffer.h"

#include <vulkan/vulkan.h>

RenderCommand* RenderCommand::create(GPUBuffer* targetBuffers, uint32_t bufferCount, uint32_t vertexCount, uint32_t indexCount) {
	RenderCommand* cmd = new RenderCommand();
	cmd->targetBuffers = targetBuffers;
	cmd->bufferCount = bufferCount;
	cmd->vertexCount = vertexCount;
	cmd->indexCount = indexCount;

	return cmd;
}

void RenderCommand::execute(VkCommandBuffer_T* commandBuffer) {
	if (targetBuffers == nullptr) {
		return;
	}

	for (uint32_t i = 0; i < bufferCount; i++) {
		targetBuffers[i].bind(commandBuffer);
	}

	vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}
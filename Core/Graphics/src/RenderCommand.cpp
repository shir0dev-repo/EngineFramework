#include "../RenderCommand.h"
#include "../Shader/GPUBuffer.h"

#include <vulkan/vulkan.h>

RenderCommand* RenderCommand::create(GPUBuffer* vertexBuffer, GPUBuffer* indexBuffer, uint32_t vertexCount, uint32_t indexCount) {
	RenderCommand* cmd = new RenderCommand();
	cmd->vertexBuffer = vertexBuffer;
	cmd->vertexCount = vertexCount;

	cmd->indexBuffer = indexBuffer;
	cmd->indexCount = indexCount;

	return cmd;
}

void RenderCommand::execute(VkCommandBuffer_T* commandBuffer) {
	if (vertexBuffer == nullptr || indexBuffer == nullptr) {
		return;
	}

	vertexBuffer->bind(commandBuffer);
	indexBuffer->bind(commandBuffer);

	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}
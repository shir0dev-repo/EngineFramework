#include "../RenderUtils.h"

#include <vulkan/vulkan.h>

void RenderUtils::makeViewport(const VkExtent2D* extent, VkViewport& viewport) {
	viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent->width;
	viewport.height = (float)extent->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
}

void RenderUtils::makeScissorRect(const VkExtent2D* extent, VkRect2D& scissor) {
	scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = *extent;
}
#pragma once

struct VkExtent2D;
struct VkViewport;
struct VkRect2D;

struct RenderUtils {
	static void makeViewport(const VkExtent2D* extent, VkViewport& viewport);
	static void makeScissorRect(const VkExtent2D* extent, VkRect2D& scissor);
};
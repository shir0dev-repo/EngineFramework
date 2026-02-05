#pragma once

typedef unsigned int uint32_t;
typedef uint32_t VkShaderStageFlags;

struct PushConstantInfo {
	uint32_t offset;
	uint32_t size;
	VkShaderStageFlags stages;
};
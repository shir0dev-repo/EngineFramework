#pragma once

typedef unsigned int uint32_t;
typedef uint32_t VkShaderStageFlags;
typedef enum VkDescriptorType;

struct DescriptorBindingInfo {
	uint32_t setIndex;
	uint32_t bindingIndex;
	VkDescriptorType type;
	uint32_t count;
	VkShaderStageFlags stages;
	char name[32];
};
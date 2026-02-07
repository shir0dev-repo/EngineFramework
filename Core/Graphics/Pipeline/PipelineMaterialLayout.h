#pragma once

typedef unsigned int uint32_t;
typedef enum VkDescriptorType;

struct MaterialBinding {
	uint32_t binding;
	VkDescriptorType type;
	const char* name;
};

struct PipelineMaterialLayout {
	uint32_t numBindings;
	MaterialBinding bindings[];
};
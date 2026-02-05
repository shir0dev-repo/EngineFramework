#pragma once

typedef unsigned int uint32_t;
typedef enum VkFormat;

struct VertexAttributeInfo {
	uint32_t location;
	VkFormat format;
	uint32_t offset;
};
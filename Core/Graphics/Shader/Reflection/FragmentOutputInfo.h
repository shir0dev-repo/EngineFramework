#pragma once

typedef unsigned int uint32_t;
typedef enum VkFormat;

struct FragmentOutputInfo {
	uint32_t location;
	VkFormat format;
};
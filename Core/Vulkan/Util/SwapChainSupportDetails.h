#pragma once

typedef unsigned int uint32_t;
typedef enum VkPresentModeKHR;

struct SwapChainSupportDetails {
	struct VkSurfaceCapabilitiesKHR* capabilities = nullptr;
	struct VkSurfaceFormatKHR* supportedFormats = nullptr;
	VkPresentModeKHR* supportedPresentModes = nullptr;

	uint32_t supportedFormatsCount = 0;
	uint32_t supportedPresentModesCount = 0;

	~SwapChainSupportDetails();
};
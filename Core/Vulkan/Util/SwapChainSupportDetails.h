#pragma once

typedef unsigned int uint32_t;
typedef enum VkPresentModeKHR;


/// <summary>Helper object for querying swapchain support info.</summary>
struct SwapChainSupportDetails {
	/// <summary>The capabilities of the swapchain.</summary>
	struct VkSurfaceCapabilitiesKHR* capabilities = nullptr;
	/// <summary>The surface formats supported by the swapchain.</summary>
	struct VkSurfaceFormatKHR* supportedFormats = nullptr;
	/// <summary>The present modes supported by the swapchain.</summary>
	VkPresentModeKHR* supportedPresentModes = nullptr;
	/// <summary>The number of formats supported by the swapchain.</summary>
	uint32_t supportedFormatsCount = 0;
	/// <summary>The number of present modes supported by the swapchain.</summary>
	uint32_t supportedPresentModesCount = 0;
	/// <summary>Cleans up SwapChainSupportDetails::supportedFormats, SwapChainSupportDetails::capabilities, and SwapChainSupportDetails::supportedPresentModes</summary>
	~SwapChainSupportDetails();
};
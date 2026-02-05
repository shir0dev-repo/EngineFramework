#pragma once

typedef unsigned int uint32_t;

/// <summary>A wrapper object for Vulkan's validation layer.</summary>
struct VulkanValidator {
#ifdef SWOS
	/// <summary>Boolean value for if the validator is currently enabled.</summary>
	const bool isValidationLayerEnabled = false;
#else
	/// <summary>Boolean value for if the validator is currently enabled.</summary>
	const bool isValidationLayerEnabled = true;
#endif
	/// <summary>Boolean value for if the validator is supported.</summary>
	bool isSupported = false;
	/// <summary>Number of available validation layers.</summary>
	uint32_t numAvailableLayers = 0;
	/// <summary>Number of enabled validation layers.</summary>
	uint32_t numEnabledLayers = 0;
	/// <summary>Number of GLFW extensions.</summary>
	uint32_t numExtensions = 0;

	/// <summary>Initializes the Vulkan validation hook.</summary>
	void setup();

	/// <summary>Gets a list of validation layer names enabled by the developer.</summary>
	/// <returns>List of enabled validation layers.</returns>
	const char* const* getValidationLayerNames() const;
	/// <summary>Gets a list of required extensions.</summary>
	/// <returns>List of required extensions.</returns>
	const char* const* getRequiredExtensions();
};
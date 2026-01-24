#pragma once

typedef unsigned int uint32_t;

struct VulkanValidator {
#ifdef NDEBUG
	const bool isValidationLayerEnabled = false;
#else
	const bool isValidationLayerEnabled = true;
#endif

	bool isSupported = false;
	uint32_t numAvailableLayers = 0;
	uint32_t numEnabledLayers = 0;
	uint32_t numExtensions = 0;

	void setup();

	const char* const* getValidationLayerNames() const;
	const char* const* getRequiredExtensions();
};
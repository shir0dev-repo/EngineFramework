#include "../SwapChainSupportDetails.h"

SwapChainSupportDetails::~SwapChainSupportDetails() {
	if (capabilities != nullptr) {
		delete capabilities;
		capabilities = nullptr;
	}
	if (supportedFormats != nullptr) {
		delete[] supportedFormats;
		supportedFormats = nullptr;
	}
	if (supportedPresentModes != nullptr) {
		delete[] supportedPresentModes;
		supportedPresentModes = nullptr;
	}

	supportedFormatsCount = 0;
	supportedPresentModesCount = 0;
}
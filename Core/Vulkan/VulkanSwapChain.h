#pragma once

typedef unsigned int uint32_t;

struct SwapChainSupportDetails;

struct GLFWwindow;

typedef enum VkPresentModeKHR;
typedef enum VkFormat;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkSurfaceKHR_T;
struct VkSwapchainKHR_T;
struct VkImage_T;
struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;
struct VkExtent2D;
struct VkImageView_T;

struct VulkanSwapChain {
	static void querySwapChainCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, SwapChainSupportDetails& supportDetails);
	
	void setup(VkPhysicalDevice_T* physicalDevice, VkDevice_T* logicalDevice, VkSurfaceKHR_T* surface,
		GLFWwindow* window);
	void teardown(VkDevice_T* logicalDevice);

private:
	void createSwapChain(VkPhysicalDevice_T* physicalDevice, VkDevice_T* logicalDevice, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void createImageViews(VkDevice_T* logicalDevice);
	void createSwapChainImageView(VkDevice_T* logicalDevice, uint32_t currentIndex);
	static void querySupportedSurfaceFormats(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceFormatKHR*& outFormats, uint32_t* count);
	static void querySupportedPresentModes(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkPresentModeKHR*& outPresentModes, uint32_t* count);
	static void querySurfaceCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceCapabilitiesKHR*& outCapabilities);

	void chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t count);
	void chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t count);
	void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	uint32_t swapChainImageCount = 0;
	uint32_t currentSwapChainImage = 0;

	SwapChainSupportDetails* supportDetails = nullptr;
	VkSwapchainKHR_T* swapChain = nullptr;

	VkFormat swapChainImageFormat;
	VkImage_T** swapChainImages = nullptr;
	VkImageView_T** swapChainImageViews = nullptr;

	VkSurfaceFormatKHR* selectedFormat = nullptr;
	VkPresentModeKHR selectedPresentMode;
	VkExtent2D* swapExtent = nullptr;
};
#pragma once

typedef unsigned int uint32_t;

struct SwapChainSupportDetails;
struct VulkanDevice;
struct Frame;

struct GLFWwindow;

typedef enum VkPresentModeKHR;
typedef enum VkFormat;

struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkSurfaceKHR_T;
struct VkSwapchainKHR_T;
struct VkRenderPass_T;
struct VkImage_T;
struct VkFramebuffer_T;
struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;
struct VkExtent2D;
struct VkImageView_T;

struct VulkanSwapChain {
	static void querySwapChainCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, SwapChainSupportDetails& supportDetails);
	
	const VkSurfaceFormatKHR* const getFormat() const { return selectedFormat; }
	const VkPresentModeKHR getPresentMode() const { return selectedPresentMode; }
	const VkExtent2D* const getExtents() const { return swapExtent; }
	VkSwapchainKHR_T* const getSwapChain() const { return swapChain; }

	Frame* const* const getFramebuffer() { return framebuffer; }
	Frame* const getCurrentFrame() { return framebuffer[currentFrameIndex]; }
	uint32_t getCurrentFrameIndex() const { return currentFrameIndex; }
	
	void incrementCurrentFrame() { currentFrameIndex = (currentFrameIndex + 1) % swapChainImageCount; }

	const uint32_t getSwapChainImageCount() const { return swapChainImageCount; }
	VkImageView_T* const getImageView(uint32_t i) const;

	void setup(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void recreate(const VulkanDevice* const device, VkRenderPass_T* renderPass, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void teardown(VkDevice_T* logicalDevice);

private:
	void createSwapChain(const VulkanDevice* const device, VkSurfaceKHR_T* surface, GLFWwindow* window);
	void createFramebuffers(const VulkanDevice* const device);

	void cleanupSwapchain(VkDevice_T* logicalDevice, bool isFinalTeardown);

	void recreateFramebuffers(const VulkanDevice* const device, VkRenderPass_T* renderPass);

	static void querySupportedSurfaceFormats(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceFormatKHR*& outFormats, uint32_t* count);
	static void querySupportedPresentModes(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkPresentModeKHR*& outPresentModes, uint32_t* count);
	static void querySurfaceCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceCapabilitiesKHR*& outCapabilities);

	void chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t count);
	void chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t count);
	void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	uint32_t swapChainImageCount = 0;
	uint32_t currentFrameIndex = 0;

	SwapChainSupportDetails* supportDetails = nullptr;
	VkSwapchainKHR_T* swapChain = nullptr;

	VkFormat swapChainImageFormat;
	VkImage_T** swapChainImages = nullptr;
	VkImageView_T** swapChainImageViews = nullptr;

	Frame** framebuffer = nullptr;

	VkSurfaceFormatKHR* selectedFormat = nullptr;
	VkPresentModeKHR selectedPresentMode;
	VkExtent2D* swapExtent = nullptr;
};
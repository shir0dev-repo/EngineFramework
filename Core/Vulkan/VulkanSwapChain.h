#pragma once

typedef unsigned int uint32_t;

struct SwapChainSupportDetails;
struct VulkanDevice;
struct Frame;
struct VulkanInstance;

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

/// <summary>A wrapper object for Vulkan's swap chain functionality.</summary>
struct VulkanSwapChain {
	/// <summary>Queries the device's swapchain capabilities .</summary>
	/// <param name="device">The physical device to query.</param>
	/// <param name="surface">The display surface this device will render to.</param>
	/// <param name="supportDetails">Out SwapChainSupportDetails for capabilities.</param>
	static void querySwapChainCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, SwapChainSupportDetails& supportDetails);
	
	/// <summary>Sets up the swapchain to be used during rendering.</summary>
	/// <param name="device">The logical device this swapchain will render to.</param>
	/// <param name="surface">The surface this swapchain will render to.</param>
	/// <param name="window">The window this swapchain will render to.</param>
	void setup(const VulkanInstance* const instance, VkSurfaceKHR_T* surface, GLFWwindow* window);
	
	/// <summary>Recreates the swapchain, resolving and out of date or suboptimal errors.</summary>
	/// <param name="device">The logical device this swapchain will render to.</param>
	/// <param name="renderPass">The render pass this swap chain is using.</param>
	/// <param name="surface">The surface this swapchain will render to.</param>
	/// <param name="window">The window this swapchain will render to.</param>
	void recreate(const VulkanInstance* const instance, VkRenderPass_T* renderPass, VkSurfaceKHR_T* surface, GLFWwindow* window);
	
	/// <summary>Cleans up any references made by the VulkanSwapChain.</summary>
	/// <param name="logicalDevice">The logical device this swapchain belonged to.</param>
	void teardown(VkDevice_T* logicalDevice);

	VkImage_T* const getImage(uint32_t index);
	VkImageView_T* const getImageView(uint32_t index);

	/// <summary>Gets the surface format of the selected device.</summary>
	/// <returns>VulkanSwapChain::selectedFormat.</returns>
	const VkSurfaceFormatKHR* const getFormat() const { return selectedFormat; }
	
	/// <summary>Gets the present mode of the selected device.</summary>
	/// <returns>VulkanSwapChain::selectedPresentMode.</returns>
	const VkPresentModeKHR getPresentMode() const { return selectedPresentMode; }
	
	/// <summary>Gets the extents of the selected device.</summary>
	/// <returns>VulkanSwapChain::swapExtent.</returns>
	const VkExtent2D* const getExtents() const { return swapExtent; }
	
	/// <summary>Gets the current swapchain instance.</summary>
	/// <returns>VulkanSwapChain::swapChain.</returns>
	VkSwapchainKHR_T* const getSwapChain() const { return swapChain; }

	/// <summary>Gets the number of swap chain images.</summary>
	/// <returns>VulkanSwapChain::swapChainImageCount.</returns>
	const uint32_t getSwapChainImageCount() const { return swapChainImageCount; }
private:
	/// <summary>Creates the VkSwapchainKHR_T*.</summary>
	/// <param name="device">The logical device this swapchain will render to.</param>
	/// <param name="surface">The surface this swapchain will render to.</param>
	/// <param name="window">The window this swapchain will render to.</param>
	void createSwapChain(const VulkanInstance* const instance, VkSurfaceKHR_T* surface, GLFWwindow* window);

	void createImages(VkDevice_T* logicalDevice);
	
	void createImageViews(VkDevice_T* logicalDevice);

	/// <summary>Cleans up any references made by the VulkanSwapChain.</summary>
	/// <param name="logicalDevice">The logical device this swapchain belongs/belonged to.</param>
	/// <param name="isFinalTeardown">Whether or not this is being called at the end of the application lifetime.</param>
	void cleanupSwapchain(VkDevice_T* logicalDevice, bool isFinalTeardown);

	/// <summary>Queries the surface's supported formats.</summary>
	/// <param name="device">The device this surface might render to.</param>
	/// <param name="surface">The queried surface.</param>
	/// <param name="outFormats">Resulting supported formats.</param>
	/// <param name="count">Number of supported formats.</param>
	static void querySupportedSurfaceFormats(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceFormatKHR*& outFormats, uint32_t* count);

	/// <summary>Queries the surface's supported present modes.</summary>
	/// <param name="device">The device this surface might render to.</param>
	/// <param name="surface">The queried surface.</param>
	/// <param name="outPresentModes">Resulting supported present modes.</param>
	/// <param name="count">Number of supported present modes.</param>
	static void querySupportedPresentModes(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkPresentModeKHR*& outPresentModes, uint32_t* count);

	/// <summary>Queries the surface's capabilities.</summary>
	/// <param name="device">The device this surface might render to.</param>
	/// <param name="surface">The queried surface.</param>
	/// <param name="outCapabilities">Resulting surface capabilities.</param>
	static void querySurfaceCapabilities(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface, VkSurfaceCapabilitiesKHR*& outCapabilities);

	/// <summary>Based on the available formats, selects the best one suited for this application.</summary>
	/// <param name="availableFormats">The formats available provided by the selected device.</param>
	/// <param name="count">The number of available formats.</param>
	void chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t count);
	
	/// <summary>Based on the available present modes, selected the best one suited for this application.</summary>
	/// <param name="availablePresentModes">The present modes available provided by the surface.</param>
	/// <param name="count">The number of available present modes.</param>
	void chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t count);
	
	/// <summary>Based on the surface's capabilities, creates an ideal extent with the dimensions of the GLFW window.</summary>
	/// <param name="capabilities">The surface's capabilities.</param>
	/// <param name="window">The GLFW app window.</param>
	void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
	
	/// <summary>The found support details when VulkanSwapChain::setup is called.</summary>
	SwapChainSupportDetails* supportDetails = nullptr;
	
	VkImage_T** swapChainImages = nullptr;
	VkImageView_T** swapChainImageViews = nullptr;

	/// <summary>The number of VkImages in the VulkanSwapChain::swapChainImages array.</summary>
	uint32_t swapChainImageCount = 0;
	
	/// <summary>The VulkanSwapChain's backing swapchain.</summary>
	VkSwapchainKHR_T* swapChain = nullptr;
	
	/// <summary>The swapchain's selected surface format.</summary>
	VkSurfaceFormatKHR* selectedFormat = nullptr;
	
	/// <summary>The swapchain's selected present mode.</summary>
	VkPresentModeKHR selectedPresentMode;
	
	/// <summary>The swapchain's extents.</summary>
	VkExtent2D* swapExtent = nullptr;
};
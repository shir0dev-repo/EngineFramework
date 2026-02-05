#pragma once

struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkInstance_T;
struct VkSurfaceKHR_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkDebugUtilsMessengerCreateInfoEXT;
struct VkApplicationInfo;
struct VkInstanceCreateInfo;

struct GLFWwindow;

struct VulkanValidator;
struct VulkanDevice;
struct VulkanSwapChain;

/// <summary>Wrapper object for the instance of Vulkan.</summary>
struct VulkanInstance {
	
	/// <summary>Vulkan validation layer.</summary>
	VulkanValidator* validator = nullptr;
	
	/// <summary>The VulkanDevice in charge of rendering.</summary>
	VulkanDevice* device = nullptr;
	
	/// <summary>The backing instance of Vulkan.</summary>
	VkInstance_T* instance = nullptr;
	
	/// <summary>The surface being rendered to.</summary>
	VkSurfaceKHR_T* surface = nullptr;
	
	/// <summary>Debug messenger sending messages to console.</summary>
	VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;
	
	/// <summary>The VulkanSwapChain.</summary>
	VulkanSwapChain* swapChain = nullptr;
	
	/// <summary>Singleton reference to VulkanInstance.</summary>
	/// <returns>VulkanInstance::instance.</returns>
	static VulkanInstance *const getInstance();
	
	/// <summary>Initializes the VulkanInstance.</summary>
	/// <param name="window">Current GLFW window handle.</param>
	/// <returns>If the creation of the VulkanInstance was successful.</returns>
	bool setup(GLFWwindow* window);
	
	bool beginSingleUseCommandBuffer(VkCommandBuffer_T** buffer) const;
	void endSingleUseCommandBuffer(VkCommandBuffer_T* buffer) const;

	/// <summary>Cleans up any underlying resources created by the VulkanInstance.</summary>
	void teardown();
private:
	/// <summary>Singleton instance of VulkanInstance.</summary>
	static VulkanInstance* vulkanInstance;
	
	/// <summary>Initializes the VulkanValidator.</summary>
	/// <returns>If the creation of the VulkanValidator was successful.</returns>
	bool setupValidator();
	
	/// <summary>Initializes the underlying Vulkan Instance.</summary>
	void setupInstance();

	/// <summary>Initializes the Vulkan Debug Messenger.</summary>
	///<param name="createInfo">The create info to initialize the debug messenger.</param>
	void setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT createInfo);

	/// <summary>Initializes the Vulkan Surface this application will render to.</summary>
	/// <param name="window">Current GLFW window handle.</param>
	void setupSurface(GLFWwindow* window);
	
	/// <summary>Initializes the VulkanDevice.</summary>
	/// <param name="window">Current GLFW window handle.</param>
	void setupDevice(GLFWwindow* window);
	
	/// <summary>Initializes the VulkanSwapChain.</summary>
	/// <param name="window">Current GLFW window handle.</param>
	void setupSwapchain(GLFWwindow* window);

	void setupGenericCommandPool();

	/// <summary>Helper function for creating the Vulkan Application Info.</summary>
	/// <param name="appInfo">The resulting application info.</param>
	void makeVkApplicationInfo(VkApplicationInfo& appInfo) const;
	
	/// <summary>Helper function for creating the Vulkan Debug Messenger Create Info.
	/// <param name="createInfo">The resulting create info.</param>
	void makeVkDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	
	/// <summary>Helper function for creating Vulkan Instance Info.</summary>
	/// <param name="appInfo">The info regarding the application.</param>
	/// <param name="createInfo">The resulting create info.</param>
	void makeVkInstanceCreateInfo(const VkApplicationInfo& appInfo, VkInstanceCreateInfo& createInfo) const;

	VkCommandPool_T* vkGenericCommandPool = nullptr;
};
#pragma once

struct VkDebugUtilsMessengerCreateInfoEXT;

struct GLFWwindow;

/// <summary>Wrapper object for the instance of Vulkan.</summary>
struct VulkanInstance {
	
	/// <summary>Vulkan validation layer.</summary>
	struct VulkanValidator* validator = nullptr;
	
	/// <summary>The VulkanDevice in charge of rendering.</summary>
	struct VulkanDevice* device = nullptr;
	
	/// <summary>The backing instance of Vulkan.</summary>
	struct VkInstance_T* instance = nullptr;
	
	/// <summary>The surface being rendered to.</summary>
	struct VkSurfaceKHR_T* surface = nullptr;
	
	/// <summary>Debug messenger sending messages to console.</summary>
	struct VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;
	
	/// <summary>The VulkanSwapChain.</summary>
	struct VulkanSwapChain* swapChain = nullptr;
	
	/// <summary>Singleton reference to VulkanInstance.</summary>
	/// <returns>VulkanInstance::instance.</returns>
	static VulkanInstance *const getInstance();
	
	/// <summary>Initializes the VulkanInstance.</summary>
	/// <param name="window">Current GLFW window handle.</param>
	/// <returns>If the creation of the VulkanInstance was successful.</returns>
	bool setup(GLFWwindow* window);
	
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


	/// <summary>Helper function for creating the Vulkan Application Info.</summary>
	/// <param name="appInfo">The resulting application info.</param>
	void makeVkApplicationInfo(struct VkApplicationInfo& appInfo) const;
	
	/// <summary>Helper function for creating the Vulkan Debug Messenger Create Info.
	/// <param name="createInfo">The resulting create info.</param>
	void makeVkDebugMessengerCreateInfo(struct VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
	
	/// <summary>Helper function for creating Vulkan Instance Info.</summary>
	/// <param name="appInfo">The info regarding the application.</param>
	/// <param name="createInfo">The resulting create info.</param>
	void makeVkInstanceCreateInfo(const struct VkApplicationInfo& appInfo, struct VkInstanceCreateInfo& createInfo) const;
};
#pragma once

typedef unsigned int uint32_t;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkInstance_T;
struct VkQueue_T;
struct VkSurfaceKHR_T;

struct GLFWwindow;

struct VulkanValidator;
struct VulkanSwapChain;
struct SwapChainSupportDetails;
struct QueueFamilyIndices;

/// <summary>Wrapper object for Vulkan's physical and logical devices.</summary>
struct VulkanDevice {
	/// <summary>The physical device that is rendering.</summary>
	VkPhysicalDevice_T* physicalDevice = nullptr;

	/// <summary>The created logical device used for rendering operations.</summary>
	VkDevice_T* logicalDevice = nullptr;

	/// <summary>The queue used to send render commands.</summary>
	VkQueue_T* graphicsQueue = nullptr;
	uint32_t graphicsQueueFamilyIndex = 0;

	/// <summary>The queue used to present the render operations to the screen.</summary>
	VkQueue_T* presentQueue = nullptr;
	uint32_t presentQueueFamilyIndex = 0;

	/// <summary>Sets up the VulkanDevice::logicalDevice based on the highest scored VkPhysicalDevice_T.</summary>
	/// <param name="instance">The current instance of Vulkan.</param>
	/// <param name="validator">The validation layer of Vulkan.</param>
	/// <param name="surface">The surface this device will render to.</param>
	/// <param name="window">Current GLFW window handle.</param>
	void setup(VkInstance_T* instance, const VulkanValidator& validator, VkSurfaceKHR_T* surface, GLFWwindow* window);

	/// <summary>Cleans up any resources created from the device.</summary>
	void teardown();

	bool deviceSupportsSamplingAnisotropy() const { return anisotropicSamplingSupported; }
private:
	/// <summary>Selects the best physical device on this machine.</summary>
	/// <param name="instance">The current Vulkan instance.</param>
	/// <param name="surface">The surface this device will render to.</param>
	void pickPhysicalDevice(struct VkInstance_T* instance, VkSurfaceKHR_T* surface);

	/// <summary>Initializes a logical device based on the selected physical device.
	/// <param name="validator">The validation layer of Vulkan.</param>
	/// <param name="surface">The surface this device will render to.</param>
	void createLogicalDevice(const VulkanValidator& validator, VkSurfaceKHR_T* surface);

	bool anisotropicSamplingSupported = false;
};
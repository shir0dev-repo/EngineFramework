#include "../VulkanInstance.h"
#include "../VulkanValidator.h"
#include "../VulkanDevice.h"
#include "../VulkanSwapChain.h"
#include "../Util/SwapChainSupportDetails.h"
#include "Core/Vulkan/Util/QueueFamilyIndices.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <map>
#include <set>

#pragma region VKDEBUG
const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

static VkResult CreateDebugUtilsMessengerEXT(
	VkInstance_T* instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

const char* const* VulkanInstance::getRequiredExtensions() {
	static std::vector<const char*> requiredGlfwExtensions;
	static uint32_t extensionsCount = 0;

	if (numGLFWExtensions > 0) {
		return requiredGlfwExtensions.data();
	}

	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&numGLFWExtensions);
	requiredGlfwExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + numGLFWExtensions);

	if (VALIDATION_LAYER_ENABLED) {
		requiredGlfwExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		numGLFWExtensions++;
	}
	extensionsCount = requiredGlfwExtensions.size();
	numGLFWExtensions = extensionsCount;

	return requiredGlfwExtensions.data();
}
#pragma endregion

#pragma region VKDEVICE
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static bool checkDeviceExtensionSupport(VkPhysicalDevice_T* device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

static uint32_t rateDeviceSuitability(VkPhysicalDevice_T* device, VkSurfaceKHR_T* surface) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	uint32_t score = 0;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = {};
	QueueFamilyIndices::findQueueFamilies(device, surface, indices);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	if (!extensionsSupported) {
		return 0;
	}

	SwapChainSupportDetails swapChainSupport = {};
	VulkanSwapChain::querySwapChainCapabilities(device, surface, swapChainSupport);
	bool swapChainAdequate = swapChainSupport.supportedFormats != nullptr && swapChainSupport.supportedPresentModes != nullptr;

	if (!deviceFeatures.geometryShader || !indices.isComplete() || !swapChainAdequate) {
		return 0;
	}

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	score += deviceProperties.limits.maxImageDimension2D;
	return score;
}

VkFormat VulkanInstance::querySupportedFormats(VkFormat* candidates, uint32_t candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features) const {
	for (uint32_t i = 0; i < candidateCount; i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return candidates[i];
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return candidates[i];
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}
#pragma endregion

VulkanInstance* VulkanInstance::vulkanInstance = nullptr;

VulkanInstance* const VulkanInstance::getInstance() {
	if (vulkanInstance == nullptr) {
		vulkanInstance = new VulkanInstance();
	}

	return vulkanInstance;
}

bool VulkanInstance::setup(GLFWwindow* window) {
	if (!setupValidator()) {
		return false;
	}

	setupInstance();
	setupSurface(window);

	setupDevice(window);
	setupSwapchain(window);
	setupGenericCommandPool();

	return true;
}

bool VulkanInstance::setupValidator() {
	if (!VALIDATION_LAYER_ENABLED) {
		isValidatorSupported = false;
		return true;
	}

	vkEnumerateInstanceLayerProperties(&numAvailableValidationLayers, nullptr);
	std::vector<VkLayerProperties> availableLayers(numAvailableValidationLayers);
	vkEnumerateInstanceLayerProperties(&numAvailableValidationLayers, availableLayers.data());
	
	isValidatorSupported = true;
	
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				numEnabledValidationLayers++;
				break;
			}
		}

		if (!layerFound) {
			isValidatorSupported = false;
		}
	}

	return isValidatorSupported;
}

void VulkanInstance::setupInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Framework";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (VALIDATION_LAYER_ENABLED) {
		createInfo.enabledLayerCount = numEnabledValidationLayers;
		createInfo.ppEnabledLayerNames = validationLayers.data();
		makeVkDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	createInfo.ppEnabledExtensionNames = getRequiredExtensions();
	createInfo.enabledExtensionCount = numGLFWExtensions;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw;
	}

	setupDebugMessenger(debugCreateInfo);
}

void VulkanInstance::setupDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT createInfo) {
	VkResult result = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &this->debugMessenger);
	if (result != VK_SUCCESS) {
		std::cerr << "Failed to setup debug messenger! Result: " << result << '\n';
		throw;
	}
}

void VulkanInstance::setupSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(instance, window, nullptr, &this->surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
}

void VulkanInstance::setupDevice(GLFWwindow* window) {
	selectPhysicalDevice();
	createLogicalDevice();
}

void VulkanInstance::selectPhysicalDevice() {
	physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("No physical devices supporting Vulkan were found!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		uint32_t score = rateDeviceSuitability(device, surface);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0) {
		this->physicalDevice = candidates.rbegin()->second;
		VkPhysicalDeviceFeatures supportedFeatures = {};
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
		if (supportedFeatures.samplerAnisotropy) {
			this->anisotropicSamplingSupported = true;
		}
	}
	else {
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

void VulkanInstance::createLogicalDevice() {
	QueueFamilyIndices queueFamily = {};
	QueueFamilyIndices::findQueueFamilies(physicalDevice, surface, queueFamily);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { queueFamily.graphicsFamily.index, queueFamily.presentFamily.index };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};

		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	if (anisotropicSamplingSupported) {
		deviceFeatures.samplerAnisotropy = VK_TRUE;
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (VALIDATION_LAYER_ENABLED) {
		deviceCreateInfo.enabledLayerCount = numEnabledValidationLayers;
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr, &this->logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	this->graphicsQueueFamilyIndex = queueFamily.graphicsFamily.index;
	this->presentQueueFamilyIndex = queueFamily.presentFamily.index;

	vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, &this->graphicsQueue);
	vkGetDeviceQueue(logicalDevice, presentQueueFamilyIndex, 0, &this->presentQueue);
}

void VulkanInstance::setupSwapchain(GLFWwindow* window) {
	this->swapChain = new VulkanSwapChain();
	this->swapChain->setup(this, this->surface, window);
}

void VulkanInstance::setupGenericCommandPool() {
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

	if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &this->vkGenericCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

void VulkanInstance::teardown() {
	if (vkGenericCommandPool != nullptr) {
		vkDestroyCommandPool(logicalDevice, vkGenericCommandPool, nullptr);
		vkGenericCommandPool = nullptr;
	}
	if (swapChain != nullptr) {
		swapChain->teardown(logicalDevice);
		delete swapChain;
		swapChain = nullptr;
	}
	if (surface != nullptr) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = nullptr;
	}
	if (instance != nullptr) {
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
	}
}

bool VulkanInstance::beginSingleUseCommandBuffer(VkCommandBuffer_T** buffer) const {
	if (this->vkGenericCommandPool == nullptr) {
		*buffer = nullptr;
		return false;
	}
	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vkGenericCommandPool;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &(*buffer)) != VK_SUCCESS) {
		return false;
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	return vkBeginCommandBuffer(*buffer, &beginInfo) == VK_SUCCESS;
}

void VulkanInstance::endSingleUseCommandBuffer(VkCommandBuffer_T* buffer) const {
	vkEndCommandBuffer(buffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(logicalDevice, vkGenericCommandPool, 1, &buffer);
}

void VulkanInstance::makeVkApplicationInfo(VkApplicationInfo& appInfo) const {
	appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Framework";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
}

void VulkanInstance::makeVkDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
}

void VulkanInstance::makeVkInstanceCreateInfo(const VkApplicationInfo& appInfo, VkInstanceCreateInfo& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	if (VALIDATION_LAYER_ENABLED) {
		createInfo.enabledLayerCount = numEnabledValidationLayers;
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	createInfo.enabledExtensionCount = numGLFWExtensions;
	createInfo.ppEnabledExtensionNames = getRequiredExtensions();
}
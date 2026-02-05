#include "../VulkanInstance.h"
#include "../VulkanValidator.h"
#include "../VulkanDevice.h"
#include "../VulkanSwapChain.h"
#include "../Util/SwapChainSupportDetails.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <iostream>

#pragma region VKDEBUG
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
	return true;
}

bool VulkanInstance::setupValidator() {
	validator = new VulkanValidator();
	validator->setup();

	if (validator->isValidationLayerEnabled && !validator->isSupported) {
		delete validator;
		return false;
	}

	return true;
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
	if (validator->isValidationLayerEnabled) {
		createInfo.enabledLayerCount = validator->numEnabledLayers;
		createInfo.ppEnabledLayerNames = validator->getValidationLayerNames();
		makeVkDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	createInfo.ppEnabledExtensionNames = validator->getRequiredExtensions();
	createInfo.enabledExtensionCount = validator->numExtensions;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		if (validator != nullptr) {
			delete validator;
		}

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
	this->device = new VulkanDevice();
	this->device->setup(this->instance, *this->validator, this->surface, window);
}

void VulkanInstance::setupSwapchain(GLFWwindow* window) {
	this->swapChain = new VulkanSwapChain();
	this->swapChain->setup(device, this->surface, window);
}

void VulkanInstance::teardown() {
	if (swapChain != nullptr) {
		swapChain->teardown(device->logicalDevice);
		delete swapChain;
		swapChain = nullptr;
	}
	if (device != nullptr) {
		device->teardown();
		delete device;
		device = nullptr;
	}
	if (validator != nullptr) {
		if (validator->isValidationLayerEnabled) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			debugMessenger = nullptr;
		}
		delete validator;
		validator = nullptr;
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
	if (this->device == nullptr || this->vkGenericCommandPool == nullptr) {
		*buffer = nullptr;
		return false;
	}
	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vkGenericCommandPool;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, &(*buffer)) != VK_SUCCESS) {
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

	vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->graphicsQueue);

	vkFreeCommandBuffers(device->logicalDevice, vkGenericCommandPool, 1, &buffer);
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

void VulkanInstance::makeVkInstanceCreateInfo(const VkApplicationInfo& appInfo, VkInstanceCreateInfo& createInfo) const {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	if (validator->isValidationLayerEnabled) {
		createInfo.enabledLayerCount = validator->numEnabledLayers;
		createInfo.ppEnabledLayerNames = validator->getValidationLayerNames();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	createInfo.enabledExtensionCount = validator->numExtensions;
	createInfo.ppEnabledExtensionNames = validator->getRequiredExtensions();
}
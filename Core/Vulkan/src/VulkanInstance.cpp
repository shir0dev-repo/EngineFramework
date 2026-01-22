#include "../VulkanInstance.h"
#include "../VulkanValidator.h"
#include "../VulkanDevice.h"

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

bool VulkanInstance::setup(GLFWwindow* window) {
	if (!setupValidator()) {
		return false;
	}

	setupInstance();
	setupDebugMessenger();
	setupSurface(window);
	setupDevice(window);
	return true;
}

void VulkanInstance::teardown() {
	if (device != nullptr) {
		device->teardown();
		device = nullptr;
	}
	if (validator->isValidationLayerEnabled) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		debugMessenger = nullptr;
	}
	if (validator != nullptr) {
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

bool VulkanInstance::setupValidator() {
	validator = new VulkanValidator();
	validator->initialize();

	if (validator->isValidationLayerEnabled && !validator->isSupported) {
		delete validator;
		return false;
	}

	return true;
}

void VulkanInstance::setupInstance() {
	VkApplicationInfo appInfo;
	makeVkApplicationInfo(appInfo);

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
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
}

void VulkanInstance::setupDebugMessenger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	makeVkDebugMessengerCreateInfo(createInfo);
	
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
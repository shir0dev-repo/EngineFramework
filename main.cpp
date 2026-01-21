#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
	if (glfwInit() != GLFW_TRUE) {
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 800, "Vulkan Test", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << '\n';

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
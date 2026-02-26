#pragma once

struct GLFWwindow;

struct AppWindow;
struct VulkanInstance;
struct Renderer;
struct GraphicsPipeline;

/// <summary>The core engine application.</summary>
struct Application {
	/// <summary>
	/// Starts the engine application.
	/// </summary>
	/// <returns>Exit code.</returns>
	int run();

	/// <summary>
	/// Currently running instance of the application.
	/// </summary>
	static Application* const getInstance();

	/// <summary>
	/// GLFW callback.
	/// </summary>
	/// <param name="window">The GLFW window handle.</param>
	/// <param name="width">The new width of the window.</param>
	/// <param name="height">The new height of the window.</param>
	static void onWindowResized(GLFWwindow* window, int width, int height);
private:
	/// <summary>
	/// Initializes GLFW window.
	/// </summary>
	/// <returns>Whether or not GLFW successfully initialized.</returns>
	bool initWindow();

	void initAssets();

	/// <summary>
	/// Initializes Vulkan requirements.
	/// </summary>
	void initVulkan();
	
	/// <summary>
	/// Initializes the application's graphics pipeline.
	/// </summary>
	void initRenderer();

	/// <summary>
	/// Initializes render materials.
	/// </summary>
	void initMaterials();
	/// <summary>
	/// Runs every frame.
	/// </summary>
	void mainLoop();
	/// <summary>
	/// Cleans up any resources created during runtime.
	/// </summary>
	void cleanup();

	/// <summary>
	/// The AppWindow of the application.
	/// </summary>
	AppWindow* window = nullptr;
	/// <summary>
	/// The VulkanInstance of the application.
	/// </summary>
	VulkanInstance* vkInstance = nullptr;
	/// <summary>
	/// The GraphicsPipeline of the application.
	/// </summary>
	Renderer* renderer = nullptr;
};
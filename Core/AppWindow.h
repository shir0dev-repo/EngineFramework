#pragma once

typedef unsigned int uint32_t;
struct GLFWwindow;

typedef void(*const FrameBufferResizeCallbackDelegate)(GLFWwindow*, int, int);

/// <summary>A wrapper object for GLFW's GLFWwindow*.</summary>
struct AppWindow {
	/// <summary> The width of the application window.</summary>
	int Width;
	/// <summary> The height of the application window.</summary>
	int Height;

	/// <summary>Gets a singleton reference to the AppWindow.</summary>
	/// <returns>The AppWindow::instance.</returns>
	static AppWindow* const getInstance();

	/// <summary>Initializes the GLFW window.</summary>
	/// <param name="width">The initial width of the window.</param>
	/// <param name="height">The initial height of the window.</param>
	/// <param name="resizeCallback">Callback for window resizing.</param>
	/// <param name="title">The title of the window.</param>
	void setup(int width, int height, FrameBufferResizeCallbackDelegate& resizeCallback, const char* title = "Vulkan Framework");
	/// <summary>Destroys the GLFW window.</summary>
	void teardown();
	/// <summary>Gets the GLFW window handle.</summary>
	/// <returns>AppWindow::instance::hWnd.</returns>
	struct GLFWwindow* const GetWindow() const;
private:
	/// <summary>The current AppWindow instance.</summary>
	static AppWindow* instance;
	/// <summary>The GLFW window handle.</summary>
	struct GLFWwindow* hWnd;
};
#pragma once

#include <map>

struct InputHandler {
	static bool getKeyDown(unsigned short keyCode);
	static bool getKey(unsigned short keyCode);
	static bool getKeyUp(unsigned short keyCode);

	static void pollKeys(struct GLFWwindow* window);
	static void GLFWkeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	static std::map<int, bool> keysPressedThisFrame;
	static std::map<int, bool> keysReleasedThisFrame;
	static std::map<int, bool> keysHeld;
};
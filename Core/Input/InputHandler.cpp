#include "InputHandler.h"
#include "Core/Events/EventHandler.h"

#include <GLFW/glfw3.h>

std::map<int, bool> InputHandler::keysPressedThisFrame{};
std::map<int, bool> InputHandler::keysReleasedThisFrame{};
std::map<int, bool> InputHandler::keysHeld{};

void InputHandler::pollKeys(GLFWwindow* window) {
	
}

void InputHandler::GLFWkeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) {
	if (action == GLFW_PRESS) {
		KeyDownEvent kde {};
		kde.keyCode = key;
		SEND_KEYBOARD_EVENT(kde);
	}
	else if (action == GLFW_RELEASE) {
		KeyUpEvent ke;
		ke.keyCode = key;
		SEND_KEYBOARD_EVENT(ke);
	}
}

bool InputHandler::getKeyDown(unsigned short keyCode) {
	if (keysPressedThisFrame.find(keyCode) == keysPressedThisFrame.end()) {
		return false;
	}

	return keysPressedThisFrame[keyCode];
}

bool InputHandler::getKey(unsigned short keyCode) {
	if (keysPressedThisFrame.find(keyCode) == keysPressedThisFrame.end()) {
		return false;
	}
	else if (keysHeld.find(keyCode) == keysHeld.end()) {
		return false;
	}

	return keysPressedThisFrame[keyCode] || keysHeld[keyCode];
}

bool InputHandler::getKeyUp(unsigned short keyCode) {
	if (keysReleasedThisFrame.find(keyCode) == keysReleasedThisFrame.end()) {
		return false;
	}
	return keysReleasedThisFrame[keyCode];
}

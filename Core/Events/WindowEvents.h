#pragma once

#include "Event.h"

enum class EWindowEvents {
	WindowResized,
	WindowClosed
};

struct WindowResizeEvent : public Event<EWindowEvents> {
	WindowResizeEvent() : Event<EWindowEvents>(EWindowEvents::WindowResized, "WindowResize") {}
	struct GLFWwindow* window;
	int width = 0;
	int height = 0;
};

struct WindowCloseEvent : public Event<EWindowEvents> {
	WindowCloseEvent() : Event<EWindowEvents>(EWindowEvents::WindowClosed, "WindowClose") {}
	virtual ~WindowCloseEvent() {}
};
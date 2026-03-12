#pragma once

#include "KeyboardEvents.h"
#include "MouseEvents.h"
#include "WindowEvents.h"
#include <memory>
#include <functional>

struct EventHandler {
	EventHandler() : WindowEventDispatcher(), KeyboardEventDispatcher(), MouseEventDispatcher() {}
	static EventHandler* getInstance();

	EventDispatcher<EWindowEvents> WindowEventDispatcher;
	EventDispatcher<EKeyboardEvents> KeyboardEventDispatcher;
	EventDispatcher<EMouseEvents> MouseEventDispatcher;

private:
	static std::unique_ptr<EventHandler> s_Instance;
};

#define ADD_WINDOW_EVENT_LISTENER(eventType, func, arg) EventHandler::getInstance()->WindowEventDispatcher.addListener(eventType, std::bind(&func, arg, std::placeholders::_1));
#define ADD_KEYBOARD_EVENT_LISTENER(eventType, func, arg) EventHandler::getInstance()->KeyboardEventDispatcher.addListener(eventType, std::bind(&func, arg, std::placeholders::_1));
#define ADD_MOUSE_EVENT_LISTENER(eventType, func, arg) EventHandler::getInstance()->MouseEventDispatcher.addListener(eventType, std::bind(&func, arg, std::placeholders::_1));

#define SEND_WINDOW_EVENT(_event) EventHandler::getInstance()->WindowEventDispatcher.sendEvent(_event);
#define SEND_KEYBOARD_EVENT(_event) EventHandler::getInstance()->KeyboardEventDispatcher.sendEvent(_event);
#define SEND_MOUSE_EVENT(_event) EventHandler::getInstance()->MouseEventDispatcher.sendEvent(_event);
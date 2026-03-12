#pragma once

#include "Event.h"

enum class EKeyboardEvents {
	KeyDown,
	KeyUp
};

struct KeyDownEvent : public Event<EKeyboardEvents> {
	KeyDownEvent() : Event<EKeyboardEvents>(EKeyboardEvents::KeyDown, "KeyDown") {}
	virtual ~KeyDownEvent() {}

	int keyCode = -1;
};

struct KeyUpEvent : public Event<EKeyboardEvents> {
	KeyUpEvent() : Event<EKeyboardEvents>(EKeyboardEvents::KeyUp, "KeyUp") {}
	virtual ~KeyUpEvent() {}

	int keyCode = -1;
};
#pragma once

#include "Event.h"

enum class EMouseEvents {
	MouseMoved,
	MouseButtonDown,
	MouseButtonUp
};

struct MouseMovedEvent : public Event<EMouseEvents> {
	MouseMovedEvent() : Event<EMouseEvents>(EMouseEvents::MouseMoved, "MouseMoved") {}
	virtual ~MouseMovedEvent() {}

	int x = -1;
	int y = -1;
};

struct MouseButtonDownEvent : public Event<EMouseEvents> {
	MouseButtonDownEvent() : Event<EMouseEvents>(EMouseEvents::MouseButtonDown, "MouseButtonDown") {}
	virtual ~MouseButtonDownEvent() {}

	int button = -1;
};

struct MouseButtonUpEvent : public Event<EMouseEvents> {
	MouseButtonUpEvent() : Event<EMouseEvents>(EMouseEvents::MouseButtonUp, "MouseButtonUp") {}
	virtual ~MouseButtonUpEvent() {}

	int button = -1;
};
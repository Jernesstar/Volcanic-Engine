#pragma once

#include "Event.h"
#include "Codes.h"

namespace VolcaniCore {

struct MouseEvent : public Event {
protected:
	MouseEvent(EventType type)
		: Event(EventCategory::Mouse, type) { }
};

struct MouseMovedEvent : public MouseEvent {
	const f64 x, y;

	MouseMovedEvent(f64 x, f64 y)
		: MouseEvent(EventType::MouseMoved), x(x), y(y) { }
};

struct MouseScrolledEvent : public MouseEvent {
	const f64 ScrollX;
	const f64 ScrollY;

	MouseScrolledEvent(f64 scrollX, f64 scrollY)
		: MouseEvent(EventType::MouseScrolled),
			ScrollX(scrollX), ScrollY(scrollY) { }
};

struct MouseButtonEvent : public MouseEvent {
	const MouseCode Button;
	const f64 x, y;

protected:
	MouseButtonEvent(EventType type, MouseCode button, f64 x, f64 y)
		: MouseEvent(type), Button(button), x(x), y(y) { }
};

struct MouseButtonPressedEvent : public MouseButtonEvent {
	MouseButtonPressedEvent(MouseCode button, f64 x, f64 y)
		: MouseButtonEvent(EventType::MouseButtonPressed, button, x, y) { }
};

struct MouseButtonReleasedEvent : public MouseButtonEvent {
	MouseButtonReleasedEvent(MouseCode button, f64 x, f64 y)
		: MouseButtonEvent(EventType::MouseButtonReleased, button, x, y) { }
};

}
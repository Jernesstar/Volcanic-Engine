#pragma once

#include "Event.h"

namespace VolcaniCore {

struct WindowEvent : public Event {
protected:
	WindowEvent(EventType type)
		: Event(EventCategory::Window, type) { }
};

struct WindowResizedEvent : public WindowEvent {
	const u32 Width, Height;

	WindowResizedEvent(u32 width, u32 height)
		: WindowEvent(EventType::WindowResized),
			Width(width), Height(height) { }
};

struct WindowMovedEvent : public WindowEvent {
	const u32 x, y;

	WindowMovedEvent(u32 x, u32 y)
		: WindowEvent(EventType::WindowMoved), x(x), y(y) { }
};

struct WindowClosedEvent : public WindowEvent {
	WindowClosedEvent()
		: WindowEvent(EventType::WindowClosed) { }
};

}

#pragma once

#include "VolcaniCore/Core/Defines.h"

#include "Window.h"
#include "Event.h"
#include "KeyEvents.h"
#include "MouseEvents.h"
#include "WindowEvents.h"
#include "ApplicationEvents.h"
#include "EventCallback.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace VolcaniCore {

template<typename TEvent>
using Callbacks = OMap<UUID, EventCallback<TEvent>>;

class Events {
public:
	static void Init();
	static void PollEvents() { glfwPollEvents(); }

	template<typename TEvent>
	requires std::derived_from<TEvent, Event>
	static void RegisterListener(const EventCallback<TEvent>& eventCallback) {
		if(!eventCallback)
			return;

		Callbacks<TEvent>& callbacks = GetCallbacks<TEvent>();
		callbacks[eventCallback.GetID()] = eventCallback;
	}

	template<typename TEvent>
	requires std::derived_from<TEvent, Event>
	static UUID RegisterListener(const Func<void,TEvent&>& callback) {
		EventCallback<TEvent> eventCallback(callback);
		RegisterListener<TEvent>(eventCallback);
		return eventCallback.GetID();
	}

	template<typename TEvent>
	requires std::derived_from<TEvent, Event>
	static void UnregisterListener(UUID callbackID) {
		GetCallbacks<TEvent>().erase(callbackID);
	}

	template<typename TEvent>
	static void Dispatch(TEvent& event) {
		Callbacks<TEvent>& callbackList = GetCallbacks<TEvent>();
	
		for(auto& [_, callback] : callbackList) {
			if(event.Handled)
				return;

			callback(event);
		}
	}

private:
	inline static Callbacks<KeyPressedEvent>          KeyPressedEventCallbacks;
	inline static Callbacks<KeyReleasedEvent>         KeyReleasedEventCallbacks;
	inline static Callbacks<KeyCharEvent>             KeyCharEventCallbacks;
	inline static Callbacks<MouseMovedEvent>          MouseMovedEventCallbacks;
	inline static Callbacks<MouseScrolledEvent>       MouseScrolledEventCallbacks;
	inline static Callbacks<MouseButtonPressedEvent>  MouseButtonPressedEventCallbacks;
	inline static Callbacks<MouseButtonReleasedEvent> MouseButtonReleasedEventCallbacks;
	inline static Callbacks<WindowResizedEvent>       WindowResizedEventCallbacks;
	inline static Callbacks<WindowMovedEvent>         WindowMovedEventCallbacks;
	inline static Callbacks<WindowClosedEvent>        WindowClosedEventCallbacks;
	inline static Callbacks<ApplicationUpdatedEvent>  ApplicationUpdatedEventCallbacks;

private:
	template<typename TEvent>
	static Callbacks<TEvent>& GetCallbacks();

	static void ErrorCallback(i32 error, const char* description);

	static void KeyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
	static void KeyCharCallback(GLFWwindow* window, u32 codepoint);
	static void MouseMovedCallback(GLFWwindow* window, f64 x, f64 y);
	static void MouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods);
	static void MouseScrolledCallback(GLFWwindow* window, f64 x_scroll, f64 y_scroll);
	static void WindowResizedCallback(GLFWwindow* window, i32 width, i32 height);
	static void WindowMovedCallback(GLFWwindow* window, i32 x, i32 y);
	static void WindowClosedCallback(GLFWwindow* window);
};

}
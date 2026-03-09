#include "Events.h"

#include "Application.h"
#include "VolcaniCore/Core/Assert.h"
#include "VolcaniCore/Core/Log.h"

#include "Input.h"

#define GET_CALLBACKS(TEvent) \
template<> \
Callbacks<TEvent>& Events::GetCallbacks<TEvent>() { \
	return TEvent##Callbacks; \
}

namespace VolcaniCore {

GET_CALLBACKS(KeyPressedEvent);
GET_CALLBACKS(KeyReleasedEvent);
GET_CALLBACKS(KeyCharEvent);
GET_CALLBACKS(MouseMovedEvent);
GET_CALLBACKS(MouseScrolledEvent);
GET_CALLBACKS(MouseButtonPressedEvent);
GET_CALLBACKS(MouseButtonReleasedEvent);
GET_CALLBACKS(WindowResizedEvent);
GET_CALLBACKS(WindowMovedEvent);
GET_CALLBACKS(WindowClosedEvent);
GET_CALLBACKS(ApplicationUpdatedEvent);

void Events::Init() {
	GLFWwindow* window = Application::GetWindow()->GetNativeWindow();

	glfwSetErrorCallback(ErrorCallback);

	glfwSetKeyCallback(window,         KeyCallback);
	glfwSetCharCallback(window,        KeyCharCallback);
	glfwSetCursorPosCallback(window,   MouseMovedCallback);
	glfwSetScrollCallback(window,      MouseScrolledCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetWindowPosCallback(window,   WindowMovedCallback);
	glfwSetWindowSizeCallback(window,  WindowResizedCallback);
	glfwSetWindowCloseCallback(window, WindowClosedCallback);
}

void Events::ErrorCallback(i32 error, const char* description) {
	// VOLCANICORE_ASSERT(false, description);
}

void Events::KeyCallback(GLFWwindow* window, i32 key, i32 scancode,
						 i32 action, i32 mods)
{
	if(action == GLFW_PRESS) {
		KeyPressedEvent event((KeyCode)key);
		Dispatch(event);
	}
	else if(action == GLFW_RELEASE) {
		KeyReleasedEvent event((KeyCode)key);
		Dispatch(event);
	}
	else if(action == GLFW_REPEAT) {
		KeyPressedEvent event((KeyCode)key, true);
		Dispatch(event);
	}
}

void Events::KeyCharCallback(GLFWwindow* window, u32 codepoint) {
	KeyCharEvent event((KeyCode)codepoint, (char)codepoint);
	Dispatch(event);
}

void Events::MouseMovedCallback(GLFWwindow* window, f64 x, f64 y) {
	MouseMovedEvent event(x, y);
	Dispatch(event);
}

void Events::MouseScrolledCallback(GLFWwindow* window,
								   f64 scrollX, f64 scrollY)
{
	MouseScrolledEvent event(scrollX, scrollY);
	Dispatch(event);
}

void Events::MouseButtonCallback(GLFWwindow* window, i32 button,
								 i32 action, i32 mods)
{
	if(action == GLFW_PRESS) {
		MouseButtonPressedEvent event((MouseCode)button,
									  Input::GetMouseX(), Input::GetMouseY());
		Dispatch(event);
	}
	if(action == GLFW_RELEASE) {
		MouseButtonReleasedEvent event((MouseCode)button,
									   Input::GetMouseX(), Input::GetMouseY());
		Dispatch(event);
	}
}

void Events::WindowResizedCallback(GLFWwindow* window, i32 width, i32 height) {
	WindowResizedEvent event((u32)width, (u32)height);
	Dispatch(event);
}

void Events::WindowMovedCallback(GLFWwindow* window, i32 x, i32 y) {
	WindowMovedEvent event((u32)x, (u32)y);
	Dispatch(event);
}

void Events::WindowClosedCallback(GLFWwindow* window) {
	WindowClosedEvent event;
	Dispatch(event);
}

}

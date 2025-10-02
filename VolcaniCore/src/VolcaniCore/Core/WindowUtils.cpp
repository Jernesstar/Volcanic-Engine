
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef VOLCANICENGINE_LINUX
#	define GLFW_EXPOSE_NATIVE_WAYLAND
#	define GLFW_EXPOSE_NATIVE_X11
#	define GLFW_EXPOSE_NATIVE_GLX
#elif VOLCANICENGINE_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif VOLCANICENGINE_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif
#include <GLFW/glfw3native.h>

namespace VolcaniCore {

void* GetNativeWindowHandle(GLFWwindow* window) {
#if VOLCANICENGINE_LINUX
	if(GLFW_PLATFORM_WAYLAND == glfwGetPlatform())
		return glfwGetWaylandWindow(window);
	return (void*)uintptr_t(glfwGetX11Window(window));
#elif VOLCANICENGINE_OSX
	return glfwGetCocoaWindow(window);
#elif VOLCANICENGINE_WINDOWS
	return glfwGetWin32Window(window);
#endif // VOLCANICENGINE_
}

void* GetNativeDisplayHandle() {
#if VOLCANICENGINE_LINUX
	if(GLFW_PLATFORM_WAYLAND == glfwGetPlatform())
		return glfwGetWaylandDisplay();
	return glfwGetX11Display();
#else
	return nullptr;
#endif // VOLCANICENGINE_*
}

}
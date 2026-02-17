#include "Window.h"

#if defined(VOLCANIC_X11)
	#include <X11/Xlib.h>
	#define GLFW_EXPOSE_NATIVE_X11
#elif defined(VOLCANIC_WAYLAND)
	#include <wayland-client.h>
	#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif defined(VOLCANIC_WINDOWS)
	#include <windows.h>
#endif

#include <GLFW/glfw3native.h>

struct WindowData {
	GLFWwindow* renderWindow;

#if defined(VOLCANIC_X11)
	Display* x11Display;
	Window parentWindow;
	Window glfwX11Window;

#elif defined(VOLCANIC_WAYLAND)
	struct wl_display* waylandDisplay;
	struct wl_surface* glfwSurface;
	struct wl_surface* parentSurface;
	struct wl_subsurface* subsurface;

	// Wayland protocol globals
	struct wl_compositor* compositor;
	struct wl_subcompositor* subcompositor;

	int currentX, currentY, currentWidth, currentHeight;
#elif defined(VOLCANIC_WINDOWS)

#endif
};

static WindowData* s_Data;

void EmbedWindow(const char* handleStr) {
	s_Data = new WindowData();
	s_Data->renderWindow = 

#if defined(VOLCANIC_X11)
	u64 windowId = std::stoull(handleStr);
	s_Data->parentWindow = static_cast<Window>(windowId);
	// Reparent the GLFW window to Tauri's window
	XReparentWindow(s_Data->x11Display, glfwX11Window, s_Data->parentWindow,
					0, 0);

	// Remove window decorations and set as child
	XSetWindowAttributes attrs;
	attrs.override_redirect = True;
	XChangeWindowAttributes(s_Data->x11Display, glfwX11Window,
							CWOverrideRedirect, &attrs);
	// Map the window
	XMapWindow(s_Data->x11Display, glfwX11Window);
	XFlush(s_Data->x11Display);
#elif defined(VOLCANIC_WAYLAND)

#elif defined(VOLCANIC_WINDOWS)

#endif
}

void UpdateBounds(u32 x, u32 y, u32 w, u32 h) {
#if defined(VOLCANIC_X11)
	XMoveResizeWindow(s_Data->x11Display, glfwX11Window, x, y, w, h);
	XFlush(s_Data->x11Display);
#elif defined(VOLCANIC_WAYLAND)

#elif defined(VOLCANIC_WINDOWS)

#endif
}

void SetVisible(bool visible) {
#if defined(VOLCANIC_X11)
	if (!s_Data->x11Display || !glfwX11Window)
		return;
	if (visible)
		XMapWindow(s_Data->x11Display, glfwX11Window);
	else
		XUnmapWindow(s_Data->x11Display, glfwX11Window);
	XFlush(s_Data->x11Display);

#elif defined(VOLCANIC_WAYLAND)

#elif defined(VOLCANIC_WINDOWS)

#endif
}
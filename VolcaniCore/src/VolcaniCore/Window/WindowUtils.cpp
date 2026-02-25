#define VOLCANIC_WAYLAND

#if defined(VOLCANIC_X11)
	#include <X11/Xlib.h>
	#define GLFW_EXPOSE_NATIVE_X11
#elif defined(VOLCANIC_WAYLAND)
	#include <wayland-client.h>
	#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif defined(VOLCANIC_WINDOWS)
	#include <windows.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef VOLCANIC_LINUX
#	define GLFW_EXPOSE_NATIVE_WAYLAND
#	define GLFW_EXPOSE_NATIVE_X11
#	define GLFW_EXPOSE_NATIVE_GLX
#elif VOLCANICENGINE_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif VOLCANIC_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif
#include <GLFW/glfw3native.h>

#include "Core/Application.h"
#include "Core/Log.h"

void* GetNativeWindowHandle(GLFWwindow* window) {
#if VOLCANIC_LINUX
	if(GLFW_PLATFORM_WAYLAND == glfwGetPlatform())
		return glfwGetWaylandWindow(window);
	return (void*)uintptr_t(glfwGetX11Window(window));
#elif VOLCANICENGINE_OSX
	return glfwGetCocoaWindow(window);
#elif VOLCANIC_WINDOWS
	return glfwGetWin32Window(window);
#endif // VOLCANICENGINE_
}

void* GetNativeDisplayHandle() {
#if VOLCANIC_LINUX
	if(GLFW_PLATFORM_WAYLAND == glfwGetPlatform())
		return glfwGetWaylandDisplay();
	return glfwGetX11Display();
#else
	return nullptr;
#endif // VOLCANICENGINE_*
}

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

	static void registry_global(void* data, struct wl_registry* registry,
							   uint32_t name, const char* interface,
							   uint32_t version)
	{
		auto* self = static_cast<WindowData*>(data);

		if(strcmp(interface, wl_compositor_interface.name) == 0) {
			self->compositor =
				static_cast<struct wl_compositor*>(
					wl_registry_bind(registry, name,
									 &wl_compositor_interface, 4));
		}
		else if(strcmp(interface, wl_subcompositor_interface.name) == 0) {
			self->subcompositor =
				static_cast<struct wl_subcompositor*>(
					wl_registry_bind(registry, name,
									 &wl_subcompositor_interface, 1)
				);
		}
	}

	static void registry_global_remove(void* data, struct wl_registry* registry,
									  uint32_t name)
	{
	}
#elif defined(VOLCANIC_WINDOWS)

#endif
};

static WindowData* s_Data;

bool EmbedWindow(const char* handleStr) {
	u64 windowID = std::stoull(handleStr);
	auto appWindow = VolcaniCore::Application::Get()->GetWindow();
	s_Data = new WindowData();
	s_Data->renderWindow = appWindow->GetNativeWindow();

#if defined(VOLCANIC_X11)
	s_Data->parentWindow = static_cast<Window>(windowID);
	s_Data->x11Display = glfwGetX11Display();
	s_Data->glfwX11Window = glfwGetX11Window(s_Data->renderWindow);

	if (!s_Data->x11Display || !s_Data->glfwX11Window) {
		VolcaniCore::Log::Info("Failed to get X11 handles");
		return false;
	}

	// Reparent the GLFW window to Tauri's window
	XReparentWindow(s_Data->x11Display, s_Data->glfwX11Window,
					s_Data->parentWindow, 0, 0);

	// Remove window decorations and set as child
	XSetWindowAttributes attrs;
	attrs.override_redirect = True;
	XChangeWindowAttributes(s_Data->x11Display, s_Data->glfwX11Window,
							CWOverrideRedirect, &attrs);
	// Map the window
	XMapWindow(s_Data->x11Display, s_Data->glfwX11Window);
	XFlush(s_Data->x11Display);

#elif defined(VOLCANIC_WAYLAND)
	// Get Wayland handles
	s_Data->parentSurface = reinterpret_cast<struct wl_surface*>(windowID);
	s_Data->waylandDisplay = glfwGetWaylandDisplay();
	s_Data->glfwSurface = glfwGetWaylandWindow(s_Data->renderWindow);

	if(!s_Data->waylandDisplay || !s_Data->glfwSurface) {
		VolcaniCore::Log::Error("Failed to get Wayland handles");
		return false;
	}

	// Get compositor and subcompositor
	struct wl_registry* registry =
		wl_display_get_registry(s_Data->waylandDisplay);
	static const struct wl_registry_listener registry_listener = {
		WindowData::registry_global,
		WindowData::registry_global_remove
	};
	wl_registry_add_listener(registry, &registry_listener, s_Data);
	wl_display_roundtrip(s_Data->waylandDisplay);

	if(!s_Data->compositor || !s_Data->subcompositor) {
		VolcaniCore::Log::Info("Failed to get required Wayland protocols");
		return false;
	}

	// Create subsurface and attach to parent
	s_Data->subsurface = wl_subcompositor_get_subsurface(
		s_Data->subcompositor, s_Data->glfwSurface, s_Data->parentSurface);

	if (!s_Data->subsurface) {
		VolcaniCore::Log::Info("Failed to create subsurface");
		return false;
	}

	// Set subsurface to desynchronized mode for better performance
	wl_subsurface_set_desync(s_Data->subsurface);
	wl_subsurface_set_position(
		s_Data->subsurface, s_Data->currentX, s_Data->currentY);

	wl_surface_commit(s_Data->parentSurface);
	wl_display_flush(s_Data->waylandDisplay);

#elif defined(VOLCANIC_WINDOWS)

#endif

	appWindow->SetEmbedded();
	return true;
}

void UpdateBounds(u32 x, u32 y, u32 w, u32 h) {
#if defined(VOLCANIC_X11)
	XMoveResizeWindow(s_Data->x11Display, s_Data->glfwX11Window, x, y, w, h);
	XFlush(s_Data->x11Display);
#elif defined(VOLCANIC_WAYLAND)
	if(!s_Data->subsurface || !s_Data->renderWindow)
		return;

	s_Data->currentX = x;
	s_Data->currentY = y;
	s_Data->currentWidth = w;
	s_Data->currentHeight = h;

	// Update subsurface position
	wl_subsurface_set_position(s_Data->subsurface, x, y);
	// Resize GLFW window
	glfwSetWindowSize(s_Data->renderWindow, w, h);

	// Commit changes
	wl_surface_commit(s_Data->glfwSurface);
	wl_surface_commit(s_Data->parentSurface);
	wl_display_flush(s_Data->waylandDisplay);
#elif defined(VOLCANIC_WINDOWS)

#endif
}

void SetVisible(bool visible) {
#if defined(VOLCANIC_X11)
	if(!s_Data->x11Display || !glfwX11Window)
		return;

	if(visible)
		XMapWindow(s_Data->x11Display, glfwX11Window);
	else
		XUnmapWindow(s_Data->x11Display, glfwX11Window);
	XFlush(s_Data->x11Display);
#elif defined(VOLCANIC_WAYLAND)
	if(!s_Data->subsurface)
		return;

	if(visible)
		wl_subsurface_place_above(s_Data->subsurface, s_Data->parentSurface);
	else
		wl_subsurface_place_below(s_Data->subsurface, s_Data->parentSurface);

	wl_surface_commit(s_Data->parentSurface);
	wl_display_flush(s_Data->waylandDisplay);
#elif defined(VOLCANIC_WINDOWS)

#endif
}

void RenderFrame() {
#if defined(VOLCANIC_WAYLAND)
	wl_display_flush(s_Data->waylandDisplay);
#endif
}

void Shutdown() {
#if defined(VOLCANIC_WAYLAND)
	if(s_Data->subsurface) {
		wl_subsurface_destroy(s_Data->subsurface);
		s_Data->subsurface = nullptr;
	}

	// if(s_Data->renderWindow) {
	// 	glfwDestroyWindow(s_Data->renderWindow);
	// 	s_Data->renderWindow = nullptr;
	// }
	
	if(s_Data->subcompositor) {
		wl_subcompositor_destroy(s_Data->subcompositor);
		s_Data->subcompositor = nullptr;
	}
	
	if(s_Data->compositor) {
		wl_compositor_destroy(s_Data->compositor);
		s_Data->compositor = nullptr;
	}
#endif
}
#include "Window.h"

#include "VolcaniCore/Core/FileUtils.h"
#include "VolcaniCore/Core/Assert.h"

#include "Application.h"
#include "Events.h"

namespace VolcanicWindow {

static GLFWwindow* CreateWindow(WindowSpecification& spec) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor* monitor = nullptr;
	const GLFWvidmode* mode = nullptr;
	if(spec.Fullscreen) {
		monitor = glfwGetPrimaryMonitor();
		mode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		spec.Width = mode->width;
		spec.Height = mode->height;
	}
	else if(spec.Undecorated) {
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	}
	else if(spec.SplashScreen) {
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Splash screen
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		// glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // Always on top
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	}

	GLFWwindow* window =
		glfwCreateWindow(spec.Width, spec.Height, spec.Title.c_str(),
						 monitor, nullptr);

	VOLCANICORE_ASSERT(window, "Could not create the window");

	if(spec.Undecorated || spec.SplashScreen) {
		monitor = glfwGetPrimaryMonitor();
		mode = glfwGetVideoMode(monitor);

		int monitorX, monitorY;
		glfwGetMonitorPos(monitor, &monitorX, &monitorY);
		glfwSetWindowPos(window,
						 monitorX + (mode->width - spec.Width) / 2,
						 monitorY + (mode->height - spec.Height) / 2);
		glfwShowWindow(window);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	return window;
}

Window::Window() {
	Events::RegisterListener<WindowResizedEvent>(
		[&](const WindowResizedEvent& event)
		{
			m_Spec.Width = event.Width;
			m_Spec.Height = event.Height;
		});
}

Window::~Window() {
	if(m_NativeWindow)
		glfwDestroyWindow(m_NativeWindow);
}

void Window::Init(const WindowSpecification& spec) {
	if(m_NativeWindow)
		glfwDestroyWindow(m_NativeWindow);

	m_Spec = spec;
	m_NativeWindow = CreateWindow(m_Spec);
	Events::Init();
}

void Window::Update() {
	glfwSwapBuffers(m_NativeWindow);
}

void Window::Maximize(bool enable) {
	m_Spec.Maximized = enable;

	if(enable)
		glfwMaximizeWindow(m_NativeWindow);
	else
		glfwRestoreWindow(m_NativeWindow);
}

void Window::Minimize() {
	m_Spec.Minimized = true;
	glfwIconifyWindow(m_NativeWindow);
}

void Window::Fullscreen(bool enable) {
	GLFWmonitor* monitor = glfwGetWindowMonitor(m_NativeWindow);
	if(enable) {
		if(monitor)
			return;

		monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(m_NativeWindow, monitor, 0, 0,
							 mode->width, mode->height,
							 mode->refreshRate);
	}
	else {
		if(!monitor)
			return;

		glfwSetWindowMonitor(m_NativeWindow, nullptr, 0, 0,
							 m_Spec.Width, m_Spec.Height, 0);
	}
}

void Window::UndoSplashScreen() {
	m_Spec.SplashScreen = false;
	glfwMakeContextCurrent(nullptr);
	Init(m_Spec);
}

void Window::Resize(uint32_t width, uint32_t height) {
	if(!(width && height) || (width == m_Spec.Width && height == m_Spec.Height))
		return;

	m_Spec.Width = width;
	m_Spec.Height = height;
	m_Spec.Maximized = false;
	glfwSetWindowSize(m_NativeWindow, width, height);
}

void Window::SetIcon(const Icon& icon) {
	if(!icon.Data) {
		glfwSetWindowIcon(m_NativeWindow, 0, nullptr);
		return;
	}

	GLFWimage iconData;
	iconData.width = icon.Width;
	iconData.height = icon.Height;
	iconData.pixels = icon.Data.Get();
	glfwSetWindowIcon(m_NativeWindow, 1, &iconData);
}

void Window::SetTitle(const std::string& title) {
	glfwSetWindowTitle(m_NativeWindow, title.c_str());
}

}
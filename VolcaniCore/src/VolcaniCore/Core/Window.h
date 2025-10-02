#pragma once

#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Buffer.h"

namespace VolcaniCore {

struct WindowSpecification {
	std::string Title;
	uint32_t Width = 800;
	uint32_t Height = 600;
	bool Undecorated = false;
	bool SplashScreen = false;
	bool Fullscreen = false;
	bool Maximized = false;
	bool Minimized = false;
};

struct Icon {
	uint32_t Width;
	uint32_t Height;
	Buffer<uint8_t> Data;
};

class Window {
public:
	Window();
	~Window();

	void Init(const WindowSpecification& spec);

	void Update();
	bool IsOpen() const { return !glfwWindowShouldClose(m_NativeWindow); }

	void Maximize(bool enable = true);
	void Minimize();
	void Fullscreen(bool enable = true);
	void UndoSplashScreen();
	void Resize(uint32_t width, uint32_t height);
	void SetIcon(const Icon& icon);
	void SetTitle(const std::string& title);

	const WindowSpecification& GetSpec() const { return m_Spec; }
	uint32_t GetWidth() const { return m_Spec.Width; }
	uint32_t GetHeight() const { return m_Spec.Height; }

	GLFWwindow* GetNativeWindow() const { return m_NativeWindow; }

private:
	WindowSpecification m_Spec;
	GLFWwindow* m_NativeWindow = nullptr;
};

extern void* GetNativeWindowHandle(GLFWwindow* window);
extern void* GetNativeDisplayHandle();

}
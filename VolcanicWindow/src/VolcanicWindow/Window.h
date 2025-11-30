#pragma once

#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "VolcaniCore/Core/Buffer.h"

using namespace VolcaniCore;

namespace VolcanicWindow {

struct WindowSpecification {
	std::string Title;
	u32 Width = 800;
	u32 Height = 600;
	u32 TickRate = 0;
	bool VSync = false;
	bool Undecorated = false;
	bool SplashScreen = false;
	bool Fullscreen = false;
	bool Maximized = false;
	bool Minimized = false;
};

struct Icon {
	u32 Width;
	u32 Height;
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
	void Resize(u32 width, u32 height);
	void SetIcon(const Icon& icon);
	void SetTitle(const std::string& title);

	const WindowSpecification& GetSpec() const { return m_Spec; }
	u32 GetWidth() const { return m_Spec.Width; }
	u32 GetHeight() const { return m_Spec.Height; }

	GLFWwindow* GetNativeWindow() const { return m_NativeWindow; }

private:
	WindowSpecification m_Spec;
	GLFWwindow* m_NativeWindow = nullptr;
};

extern void* GetNativeWindowHandle(GLFWwindow* window);
extern void* GetNativeDisplayHandle();

}
#pragma once

#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Core/Buffer.h"

namespace VolcaniCore {

struct WindowSpecification {
	std::string Title = "";
	u32 Width = 800;
	u32 Height = 600;
	u32 TickRate = 0;
	bool VSync = false;
	bool Undecorated = false;
	bool SplashScreen = false;
	bool Fullscreen = false;
	bool Maximized = false;
	bool Minimized = false;
	bool Embedded = false;
	bool Hidden = false;
};

struct Icon {
	u32 Width;
	u32 Height;
	Buffer<u8> Data;
};

class Window {
public:
	Window();
	~Window();

	void Init(const WindowSpecification& spec);

	void Update();
	bool IsOpen() const { return !glfwWindowShouldClose(m_NativeWindow); }

	void SetEmbedded();
	void Maximize(bool enable = true);
	void Minimize();
	void Fullscreen(bool enable = true);
	void UndoSplashScreen();
	void Resize(u32 width, u32 height);
	void SetIcon(const Icon& icon);
	void SetTitle(const std::string& title);

	u32 GetWidth() const;
	u32 GetHeight() const;

	const WindowSpecification& GetSpec() const { return m_Spec; }

	GLFWwindow* GetNativeWindow() const { return m_NativeWindow; }

private:
	WindowSpecification m_Spec;
	GLFWwindow* m_NativeWindow = nullptr;
};

}

void* GetNativeWindowHandle(GLFWwindow* window);
void* GetNativeDisplayHandle();
bool EmbedWindow(const char* handleStr);
void UpdateBounds(u32 x, u32 y, u32 w, u32 h);
void SetVisible(bool visible);
void RenderFrame();
void Shutdown();

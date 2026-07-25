#pragma once

#include "Core/TimeUtils.h"
#include "Core/Defines.h"

#include "Window/Window.h"

int main(int argc, char** argv);

namespace VolcaniCore {

struct AppSpecification {
	std::string Name;
	u32 TickRate = 0; // = 0 => Fast as possible; = n => n ticks per sec
};

class Application {
public:
	Application(const AppSpecification& spec = { "Application" },
				const WindowSpecification& windowSpec = { "Application", 1280, 720 });
	virtual ~Application() = default;

	virtual void OnUpdate(TimeStep ts) = 0;

protected:
	virtual void Run();
	inline static TimePoint m_LastFrame{ Time::GetTime() };

public:
	// Reset the frame timer so the next frame's ts starts from now. Call when
	// entering play mode to avoid handing the entire editor-idle duration to
	// the first play frame.
	static void ResetFrameTimer() { m_LastFrame = Time::GetTime(); }
protected:

	inline static Application* s_Instance;
	inline static AppSpecification s_Spec;
	inline static Ref<Window> s_Window;
public:
	static void Close(u32 code = 0);
	static Application* Get();
	static Ref<Window> GetWindow() { return s_Window; }

	static std::string GetHomeDir();
	static std::string GetCurrentDir();
	static std::string GetLibraryDir();
	static void PushDir();
	static void PushDir(const std::string& path);
	static void PopDir();

	template<typename TDerived>
	requires std::derived_from<TDerived, Application>
	static TDerived* As() { return (TDerived*)Get(); }

private:
	static void SetCurrentDir();

	inline static std::string s_LibraryPath;
	inline static std::string s_Path;

	friend int ::main(int argc, char** argv);
};

}
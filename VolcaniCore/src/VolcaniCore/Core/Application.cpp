#include <cstdlib>
#include <chrono>
#include <thread>
#include <algorithm>

#ifdef VOLCANIC_LINUX
	#include <stdlib.h>
#endif

#include "Application.h"
#include "Assert.h"

#include "Window/Events.h"

namespace fs = std::filesystem;

namespace VolcaniCore {

void Application::Close(u32 code) {
	delete s_Instance;

	s_Window.reset();
	glfwTerminate();

	exit(code);
}

Application* Application::Get() {
	return s_Instance;
}

Application::Application(const AppSpecification& spec,
						 const WindowSpecification& windowSpec)
{
	s_Instance = this;
	s_Spec = spec;

	unsetenv("WAYLAND_DISPLAY");
	unsetenv("XDG_SESSION_TYPE");
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

	VOLCANICORE_ASSERT(glfwInit(), "Failed to initialize GLFW");

	s_Window = CreateRef<Window>();
	s_Window->Init(windowSpec);
}

void Application::Run() {
	while(s_Window->IsOpen()) {
		TimePoint time = Time::GetTime();
		TimeStep ts = time - m_LastFrame; // SECONDS
		m_LastFrame = time;

		// Clamp the ts handed to OnUpdate so a huge hitch (window drag, asset
		// load, editor idle) can't blow up per-frame catch-up loops. Keep the
		// raw ts for the sleep math below.
		f32 rawTs = (f32)ts;
		TimeStep updateTs = std::min(rawTs, 0.1f);

		Events::PollEvents();
		s_Instance->OnUpdate(updateTs);
		s_Window->Update();

		if(!s_Spec.TickRate)
			continue;

		f32 targetDelta = 1.0f / f32(s_Spec.TickRate); // SECONDS
		if(rawTs < targetDelta) {
			f32 sleep = (targetDelta - rawTs) * 1000.0f; // → milliseconds
			auto timeMS = std::chrono::milliseconds(static_cast<u32>(sleep));
			std::this_thread::sleep_for(timeMS);
		}
	}
}

static std::string s_OldPath;

std::string Application::GetHomeDir() {
#ifdef VOLCANIC_WINDOWS
	return getenv("USERPROFILE");
#elif VOLCANIC_LINUX
	return getenv("HOME");
#endif
}

std::string Application::GetCurrentDir() {
	return s_Path;
}

std::string Application::GetLibraryDir() {
	return s_LibraryPath;
}

void Application::PushDir() {
	PushDir(s_LibraryPath);
}

void Application::PushDir(const std::string& path) {
	if(path == "")
		return;

	s_OldPath = s_Path;
	s_Path = path;
	try {
		fs::current_path(s_Path);
	}
	catch(const std::exception& e) {
		Log::Error("Failed to push directory '{}': {}", s_Path, e.what());
	}
}

void Application::PopDir() {
	try {
		s_Path = s_OldPath;
		fs::current_path(s_Path);
	}
	catch(const std::exception& e) {
		Log::Error("Failed to pop directory '{}': {}", s_Path, e.what());
	}
}

void Application::SetCurrentDir() {
	s_Path = fs::current_path().string();
	s_OldPath = s_Path;

	char* env = getenv("VOLC_PATH");
	if(!env) {
		Log::Warning("Cound not find VOLC_PATH env variable");
		return;
	}

	s_LibraryPath = env;
}

}

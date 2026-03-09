#include <cstdlib>
#include <chrono>
#include <thread>

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

	VOLCANICORE_ASSERT(glfwInit(), "Failed to initialize GLFW");

	s_Window = CreateRef<Window>();
	s_Window->Init(windowSpec);
}

void Application::Run() {
    f32 targetDelta = (1.0f / (f32)s_Spec.TickRate) * 1000.0f;
    f32 accumulator = 0.0f;

    while(s_Window->IsOpen()) {
        TimePoint time = Time::GetTime();
        TimeStep ts = time - m_LastFrame;
        m_LastFrame = time;
        accumulator += ts;

        Events::PollEvents();

        // Fixed update catch-up
        while(accumulator >= targetDelta) {
            s_Instance->OnUpdate(targetDelta); // Logic uses fixed step
            accumulator -= targetDelta;
        }

        s_Window->Update(); // Render as fast as possible or sync to VSync
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
	fs::current_path(s_Path);
}

void Application::PopDir() {
	s_Path = s_OldPath;
	fs::current_path(s_Path);
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

#include <cstdlib>
#include <chrono>
#include <thread>

#include "Application.h"

#include "VolcaniCore/Core/Assert.h"

#include "Events.h"

namespace VolcanicWindow {

WindowApplication::WindowApplication(const WindowSpecification& spec)
	: Application({ spec.Title, spec.TickRate })
{
// #ifdef VOLCANIC_LINUX
// 	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
// #endif

	VOLCANICORE_ASSERT(glfwInit(), "Failed to initialize GLFW");

	m_Window = CreateRef<Window>();
	m_Window->Init(spec);
}

WindowApplication::~WindowApplication() {
	m_Window.reset();
	// glfwTerminate();
}

void WindowApplication::Run() {
	while(m_Window->IsOpen()) {
		TimePoint time = Time::GetTime();
		TimeStep ts = time - m_LastFrame;
		m_LastFrame = time;

		Events::PollEvents();

		s_Instance->OnUpdate(ts);

		m_Window->Update();

		if(!s_Spec.TickRate)
			continue;

		f32 targetDelta = (1.0f / float(s_Spec.TickRate)) * 1000.0f;
		if (ts < targetDelta) {
			f32 sleep = targetDelta - ts;
			auto timeMS = std::chrono::milliseconds(static_cast<u32>(sleep));
			std::this_thread::sleep_for(timeMS);
		}
	}
}

}

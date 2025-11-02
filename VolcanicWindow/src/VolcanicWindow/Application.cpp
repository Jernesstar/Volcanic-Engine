#include <cstdlib>

#include "Application.h"

#include "VolcaniCore/Core/Assert.h"

#include "Events.h"

namespace VolcanicWindow {

WindowApplication::WindowApplication(const WindowSpecification& spec)
	: Application({ spec.Title })
{
	VOLCANICORE_ASSERT(glfwInit(), "Failed to initialize GLFW");

	m_Window = CreateRef<Window>();
	m_Window->Init(spec);
}

WindowApplication::~WindowApplication() {
	m_Window.reset();
	glfwTerminate();
}

void WindowApplication::Run() {
	while(m_Window->IsOpen()) {
		TimePoint time = Time::GetTime();
		TimeStep ts = time - m_LastFrame;
		m_LastFrame = time;

		Events::PollEvents();

		s_Instance->OnUpdate(ts);

		m_Window->Update();
	}
}

}

#pragma once

#include "VolcaniCore/Core/Application.h"

#include "Window.h"

namespace VolcanicWindow {

class WindowApplication : public VolcaniCore::Application {
public:
	WindowApplication(const WindowSpecification& spec);
	~WindowApplication();

	Ref<Window> GetWindow() { return m_Window; }

private:
	Ref<Window> m_Window;

private:
	void Run() override;
};

}
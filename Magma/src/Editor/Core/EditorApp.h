#pragma once

#include <VolcanicWindow/Application.h>
#include <VolcaniCore/Core/CommandLineArgs.h>

#include "Editor.h"

using namespace VolcaniCore;

namespace Magma {

class EditorApp : public VolcanicWindow::WindowApplication {
public:
	EditorApp(const CommandLineArgs& args);
	~EditorApp();

	void OnUpdate(TimeStep ts) override;

	Editor& GetEditor() { return m_Editor; }

private:
	Editor m_Editor;
};

}
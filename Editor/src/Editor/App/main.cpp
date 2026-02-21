#include <VolcaniCore/Core/CommandLineArgs.h>

#include "App.h"

Application* CreateApplication(const CommandLineArgs& args) {
	return new VolcanicEditor::EditorApp(args);
}
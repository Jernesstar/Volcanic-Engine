#include <VolcaniCore/Core/CommandLineArgs.h>

#include "Core/EditorApp.h"

Application* CreateApplication(const CommandLineArgs& args) {
	return new Magma::EditorApp(args);
}
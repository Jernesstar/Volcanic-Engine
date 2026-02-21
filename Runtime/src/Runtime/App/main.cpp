#include <VolcaniCore/Core/CommandLineArgs.h>

#include "App.h"

Application* CreateApplication(const CommandLineArgs& args) {
	return new VolcanicRuntime::RuntimeApp(args);
}
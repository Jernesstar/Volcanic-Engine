#pragma once

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/CommandLineArgs.h>

using namespace VolcaniCore;

namespace VolcanicRuntime {

class RuntimeApp : public Application {
public:
	RuntimeApp(const CommandLineArgs& args);
	~RuntimeApp();

	void OnUpdate(TimeStep ts) override;
};

}
#pragma once

#include <VolcanicWindow/Application.h>
#include <VolcaniCore/Core/CommandLineArgs.h>

#include <Lava/Core/App.h>

using namespace VolcaniCore;
using namespace VolcanicWindow;

namespace Lava {

class Runtime : public WindowApplication {
public:
	Runtime(const CommandLineArgs& args);
	~Runtime();

	void OnUpdate(TimeStep ts) override;

public:
	Ref<App> m_App;
};

}
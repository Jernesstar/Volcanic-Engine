#pragma once

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/CommandLineArgs.h>

#include <Engine/Core/App.h>

using namespace VolcaniCore;

namespace Lava {

class Runtime : public Application {
public:
	Runtime(const CommandLineArgs& args);
	~Runtime();

	void OnUpdate(TimeStep ts) override;

public:
	Ref<App> m_App;
};

}
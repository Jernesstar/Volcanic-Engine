#pragma once

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/CommandLineArgs.h>

using namespace VolcaniCore;

namespace VolcanicEditor {

class EditorApp : public Application {
public:
	EditorApp(const CommandLineArgs& args);
	~EditorApp();

	void OnUpdate(TimeStep ts) override;
};

}
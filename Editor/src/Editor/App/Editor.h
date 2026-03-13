#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/CommandLineArgs.h>
#include <VolcaniCore/Core/TimeUtils.h>

#include <Engine/App/Project.h>

using namespace VolcaniCore;
using namespace VolcanicEngine;

namespace VolcanicEditor {

class Editor {
public:
	static void Init(const CommandLineArgs& args);
	static void Close();

	static void Update(TimeStep ts);
	static void Render();

	static void OnPlay(bool debug = false);
	static void OnPause();
	static void OnResume();
	static void OnStop();

	static Project& GetProject();
	static void OpenProject(const Str& path);
	static void NewProject(const Str& path);
	static void SaveProject();

	static void NewScene(const Str& name);
	static void OpenScene(const Str& name);
	static void SaveScene(const Str& name);

	static void NewCanvas(const Str& name);
	static void OpenCanvas(const Str& name);
	static void SaveCanvas(const Str& name);
};

}

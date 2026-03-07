#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/CommandLineArgs.h>
#include <VolcaniCore/Core/TimeUtils.h>

using namespace VolcaniCore;

namespace VolcanicEditor {

class Editor {
public:
	static void Init(const CommandLineArgs& args);
	static void Close();

	static void Update(TimeStep ts);
	static void Render();

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
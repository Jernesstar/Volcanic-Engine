#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/CommandLineArgs.h>
#include <VolcaniCore/Core/TimeUtils.h>

#include <Engine/App/Project.h>
#include <Engine/ECS/World.h>

using namespace VolcaniCore;
using namespace VolcanicEngine;

namespace VolcanicEditor {

enum class EditorMode { Edit, Preview, Play, Pause };

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
	static EditorMode GetMode();

	static void OpenProject(const Str& path);
	static void NewProject(const Str& path);
	static void SaveProject();

	static void NewScene(const Str& name);
	static void OpenScene(const Str& name);
	static void SaveScene(const Str& name);
	static void SaveScene();

	// Selection — shared across panels
	static ECS::Entity GetSelected();
	static void SetSelected(ECS::Entity entity);
	static void ClearSelected();
};

}
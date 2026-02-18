#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Project.h"
#include "ECS/World.h"
#include "Script/ScriptModule.h"

// #include "UI/UI.h"
// #include "SceneRenderer.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Script;
// using namespace VolcanicEngine::UI;

namespace VolcanicEngine {

class App {
public:
	static App* Get() { return s_Instance; }

public:
	// bool ChangeScreen;
	// bool RenderScene;
	// bool RenderUI;
	bool Running;

	// Func<void, Ref<ScriptModule>&> AppLoad;

public:
	App();
	~App() = default;

	void OnLoad();
	void OnClose();
	void OnUpdate(TimeStep ts);

	void SetProject(const Project& project) { m_Project = project; }
	Project& GetProject() { return m_Project; }

private:
	Project m_Project;

private:
	inline static App* s_Instance;
};

}
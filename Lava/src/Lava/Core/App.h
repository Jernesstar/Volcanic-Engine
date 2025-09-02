#pragma once

#include <VolcaniCore/Core/Defines.h>

#include <Magma/Core/Project.h>
#include <Magma/Script/ScriptModule.h>

#include "ECS/World.h"
// #include "UI/UI.h"
// #include "SceneRenderer.h"

using namespace VolcaniCore;
using namespace Magma;
using namespace Magma::Script;
// using namespace Lava::UI;

namespace Lava {

class App {
public:
	static App* Get() { return s_Instance; }

public:
	// bool ChangeScreen;
	// bool RenderScene;
	// bool RenderUI;
	bool Running;

	Func<void, Ref<ScriptModule>&> AppLoad;

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
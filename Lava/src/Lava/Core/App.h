#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/TimeUtils.h>

#include <Magma/Core/Project.h>
#include <Magma/Script/ScriptModule.h>

using namespace VolcaniCore;
using namespace Magma;
using namespace Magma::Script;

namespace Lava {

class App {
public:
	static App* Get() { return s_Instance; }

public:
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
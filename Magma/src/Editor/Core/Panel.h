#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/TimeUtils.h>

#include <Lava/Script/ScriptObject.h>

using namespace VolcaniCore;
using namespace Lava::Script;

namespace Magma::UI {

class Tab;

class Panel {
public:
	static void RegisterInterface();

public:
	const std::string Name;
	bool Open = false;

public:
	Panel(const std::string& tab, const std::string& name);
	~Panel();

	void OnOpen();
	void OnClose();
	void OnUpdate(TimeStep ts);
	void OnRender();

private:
	Tab* m_Tab = nullptr;
	asILockableSharedBool* m_IsDead = nullptr;
	ScriptObject m_ScriptObj;

private:
	Panel* GetPanel(const std::string& name);
};

}
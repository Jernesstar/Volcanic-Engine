#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Time.h>

#include <Magma/Script/ScriptObject.h>

using namespace VolcaniCore;
using namespace Magma::Script;

namespace Magma {

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
	ScriptObject m_ScriptObj;
	asILockableSharedBool *m_IsDead;

private:
	Panel* GetPanel(const std::string& name);
};

}
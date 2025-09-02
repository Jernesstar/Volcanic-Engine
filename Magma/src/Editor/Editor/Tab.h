#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/Time.h>

#include <Magma/Script/ScriptObject.h>

#include "Panel.h"

using namespace VolcaniCore;
using namespace Magma::Script;

namespace Magma {

class Tab {
public:
	static void RegisterInterface();

public:
	std::string Type;
	std::string Name;
	List<Panel> Panels;

public:
	Tab();
	~Tab();

	void Init(ScriptObject obj);

	void OnLoad(const std::string& path);
	void OnSelect();
	void OnDeselect();
	void OnClose();
	void OnUpdate(TimeStep ts);
	void OnRender();

	Panel* GetPanel(const std::string& name);

private:
	// The C++ side holds a weak link to the script side to
	// avoid a circular reference between the C++ side and script side
	asILockableSharedBool *m_IsDead;
	ScriptObject m_ScriptObj;
};

}
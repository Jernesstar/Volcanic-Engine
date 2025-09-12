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
	std::string Type = "None";
	std::string Name = "Empty";
	List<Panel> Panels;

public:
	Tab();
	~Tab();
	// Tab(Tab&&) = default;
	// Tab(const Tab&) = delete;

	// Tab& operator =(Tab&&) = default;
	// Tab& operator =(const Tab&) = delete;

	void Init(const std::string& type);

	void OnOpen();
	void OnClose();
	void OnLoad(const std::string& path = "");

	void OnSelect();
	void OnDeselect();

	void OnUpdate(TimeStep ts);
	void OnRender();

	Panel* GetPanel(const std::string& name);

private:
	// The C++ side holds a weak link to the script side to
	// avoid a circular reference between the C++ side and script side
	asILockableSharedBool *m_IsDead = nullptr;
	ScriptObject m_ScriptObj;
};

}
#pragma once

#include <VolcaniCore/Core/CommandLineArgs.h>

#include <Lava/Core/App.h>
#include <Lava/Core/Project.h>

#include "Integration/AI/AI.h"

#include "Tab.h"

using namespace VolcaniCore;
using namespace Lava;

namespace Magma {

struct Component {
	std::string Name;
	std::string Path;
	bool BuildAuto;
	List<std::string> CoreDeps;
	List<std::string> EditorDeps;
};

struct LavaFlow {
	std::string Name;
	std::string Path;
	List<std::string> Components;
	List<std::string> ObjectList;
};

struct Cache {
	List<std::string> PastProjects;
	List<std::string> PastLavaFlows;
};

class Editor {
public:
	static void RegisterInterface();

public:
	enum class EditorMode { 
		None, Component, Flow, Project
	} Mode = EditorMode::None;

public:
	Editor() { s_Instance = this; }
	~Editor() = default;

	void Open();
	void Close();

	void Load(const CommandLineArgs& args);
	void Update(TimeStep ts);
	void Render();

	void NewTab();
	void OpenTab(const std::string& type);
	void LoadTab(const std::string& type = "None");
	void ReopenTab();
	void CloseTab(uint32_t idx);
	void SetTab(uint32_t idx);

	static Ref<ScriptModule> GetModule(const std::string& name);
	static Ref<ScriptClass> GetTabClass(const std::string& name);
	static Ref<ScriptClass> GetPanelClass(const std::string& tab,
										  const std::string& name);

	static UI::Tab* GetCurrentTab() {
		if(!s_Instance->m_CurrentTab)
			return nullptr;
		return s_Instance->m_Tabs.At(s_Instance->m_CurrentTab - 1);
	}

	static const LavaFlow& GetLavaFlow() { return s_Instance->m_LavaFlow; }
	static Project& GetProject() { return s_Instance->m_Project; }
	static Ref<Lava::App> GetApp() { return s_Instance->m_App; }

private:
	inline static Editor* s_Instance;

private:
	Ref<Lava::App> m_App;

	Project m_Project;
	LavaFlow m_LavaFlow;
	Component m_Component;

	Cache m_Cache;
	uint64_t m_CurrentTab = 0;
	List<UI::Tab> m_Tabs;
	List<UI::Tab> m_ClosedTabs;

	void RenderStartScreen();
	void RenderComponentEditor();
	void RenderFlowEditor();
	void RenderProjectEditor();

	void NewProject();
	void OpenProject();
	void OpenProject(const std::string& file);
	void CloseProject();

	void NewLavaFlow();
	void OpenLavaFlow();
	void OpenLavaFlow(const std::string& file);
	void CloseLavaFlow();

	void NewComponent();
	void OpenComponent();
	void OpenComponent(const std::string& file);
	void CloseComponent();
};

}
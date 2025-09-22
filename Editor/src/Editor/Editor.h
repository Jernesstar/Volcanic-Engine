#pragma once

#include <VolcaniCore/Core/CommandLineArgs.h>

#include <Magma/Core/Project.h>

#include <Lava/Core/App.h>

#include "Tab.h"

using namespace VolcaniCore;

namespace Magma {

struct LavaFlow {
	std::string Name;
	std::string Path;
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
	Editor() { s_Instance = this; }
	~Editor() = default;

	void Open();
	void Close();

	void Load(const CommandLineArgs& args);
	void Update(TimeStep ts);
	void Render();

	static Ref<ScriptModule> GetModule(const std::string& name);
	static Ref<ScriptClass> GetTabClass(const std::string& name);
	static Ref<ScriptClass> GetPanelClass(const std::string& tab,
										  const std::string& name);

	static Tab* GetCurrentTab() {
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
	Cache m_Cache;

	uint64_t m_CurrentTab = 0;
	List<Tab> m_Tabs;
	List<Tab> m_ClosedTabs;

	void RenderStartScreen();
	void RenderTitleBar();

	void NewTab();
	void OpenTab(const std::string& type);
	void LoadTab(const std::string& type = "None");
	void ReopenTab();
	void CloseTab(uint32_t idx);
	void SetTab(uint32_t idx);

	void NewProject();
	void NewProject(const std::string& volcPath);
	void OpenProject();
	void RunProject();
	void CloseProject();
	void ExportProject();
	void ExportProject(const std::string& path);
	void NewLavaFlow(const std::string& path);
	void LoadLavaFlow(const std::string& path);
	void CloseLavaFlow();
};

}
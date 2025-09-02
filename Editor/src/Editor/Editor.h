#pragma once

#include <VolcaniCore/Core/CommandLineArgs.h>

#include <Magma/Core/Project.h>

#include <Lava/Core/App.h>
#include <Lava/Component/Ash/AssetManager.h>

#include "Tab.h"

using namespace VolcaniCore;

namespace Magma {

struct LavaFlow {
	std::string Path;
	std::string Name;
	List<std::string> ObjectList;
};

class Editor {
public:
	Editor() { s_Instance = this; }
	~Editor() = default;

	void Open();
	void Close();
	
	void Load(const CommandLineArgs& args);
	void Update(TimeStep ts);
	void Render();

	static Tab* GetCurrentTab() {
		if(!s_Instance->m_CurrentTab)
			return nullptr;
		return s_Instance->m_Tabs.At(s_Instance->m_CurrentTab - 1);
	}

	static Project& GetProject() { return s_Instance->m_Project; }
	static Ref<Lava::App> GetApp() { return s_Instance->m_App; }

private:
	inline static Editor* s_Instance;

private:
	Project m_Project;
	LavaFlow m_LavaFlow;
	Ref<Lava::App> m_App;
	// EditorAssetManager m_AssetManager;

	uint64_t m_CurrentTab = 0;
	List<Tab> m_Tabs;
	List<Tab> m_ClosedTabs;

	void SetTab(uint32_t idx);
	void NewTab();
	void NewTab(Tab tab);
	void OpenTab(const std::string& type);
	void LoadTab(const std::string& type = "None");
	void ReopenTab();
	void CloseTab(uint32_t idx);

	void NewProject();
	void NewProject(const std::string& volcPath);
	void OpenProject();
	void RunProject();
	void CloseProject();
	void ExportProject();
	void ExportProject(const std::string& path);
	void LoadLavaFlow(const std::string& path);

	void RenderEmptyTab(Tab& tab);
	void RenderWelcomeScreen();
};

}
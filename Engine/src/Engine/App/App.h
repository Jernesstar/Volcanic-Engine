#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Project.h"
#include "ECS/World.h"
#include "Script/ScriptModule.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Script;
namespace VolcanicEngine {

class App {
public:
	static App* Get() { return s_Instance; }

public:
	bool ChangeScreen;
	bool RenderScene;
	bool Running;

	Func<void, Ref<ScriptModule>&> AppLoad;
	Func<void, Ref<ScriptModule>&, const std::string&> ScreenLoad;
	Func<void, Scene&> SceneLoad;
	Func<void, const std::string&> Log;

public:
	App();
	~App() = default;

	void PrepareScreen();
	void OnLoad();
	void OnClose();
	void OnUpdate(TimeStep ts);

	void LoadScene(Scene* scene);
	Scene* GetScene();

	void SwitchScreen(const std::string& name);
	void PushScreen(const std::string& name);
	void PopScreen(const std::string& name);

	void ScreenPush(const std::string& name);
	void ScreenSet(const std::string& name);
	void ScreenPop();

	void SetProject(const Project& project) { m_Project = project; }
	Project& GetProject() { return m_Project; }

	void CreateSceneRenderer();
	Ref<RuntimeSceneRenderer> GetSceneRenderer() { return m_SceneRenderer; }

	void SetOutputPass(Ref<RenderPass> pass) { m_OutputPass = pass; }

private:
	Project m_Project;
	Ref<RenderPass> m_OutputPass;
	Ref<RuntimeSceneRenderer> m_SceneRenderer;

private:
	inline static App* s_Instance;
};

}
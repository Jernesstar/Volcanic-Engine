#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Project.h"
#include "ECS/World.h"
#include "Script/ScriptModule.h"
#include "Scene/Scene.h"
#include "Scene/Graphics/SceneRenderer.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Script;

namespace VolcanicEngine {

class App {
public:
	static App* Get() { return s_Instance; }

public:
	bool Running = false;

	Func<Ref<ScriptModule>> AppLoad;
	Func<void, Scene&> SceneLoad;
	Func<void, const std::string&> Log;

public:
	App();
	~App();

	void OnLoad();
	void OnClose();
	void ReleaseScriptModule();
	void OnUpdate(TimeStep ts);

	void LoadScene(Scene* scene);
	Scene* GetScene();

	void SetProject(const Project& project) { m_Project = project; }
	Project& GetProject() { return m_Project; }

	SceneRenderer& GetSceneRenderer() { return m_SceneRenderer; }
	Ref<Framebuffer> GetRenderOutput() { return m_SceneRenderer.GetOutput(); }

	void UseDefaultPipeline();
	void UseDefaultPipeline(u32 renderW, u32 renderH);
	void AddRenderHook(asIScriptObject* obj);
	void RemoveRenderHook(asIScriptObject* obj);
	void SetPipeline(asIScriptObject* pipelineObj);

private:
	Project m_Project;
	SceneRenderer m_SceneRenderer;

	inline static App* s_Instance = nullptr;
};

}
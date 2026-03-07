#pragma once

#include <VolcaniCore/Core/Defines.h>

#include "Project.h"
#include "ECS/World.h"
#include "Script/ScriptModule.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Canvas/Canvas.h"
#include "Canvas/CanvasRenderer.h"

using namespace VolcaniCore;
using namespace VolcanicEngine;
using namespace VolcanicEngine::Script;
// using namespace VolcanicEngine::UI;

namespace VolcanicEngine {

class App {
public:
	static App* Get() { return s_Instance; }

public:
	bool ChangeScreen;
	bool RenderScene;
	bool RenderUI;
	bool Running;

	Func<void, Ref<ScriptModule>&> AppLoad;
	Func<void, Ref<ScriptModule>&, const std::string&> ScreenLoad;
	Func<void, Scene&> SceneLoad;
	Func<void, Canvas&> UILoad;
	Func<void, const std::string&> Log;

public:
	App();
	~App() = default;

	void PrepareScreen();
	void OnLoad();
	void OnClose();
	void OnUpdate(TimeStep ts);

	void LoadScene(Scene* scene);
	void LoadCanvas(Canvas* canvas);
	Scene* GetScene();
	Canvas* GetCanvas();

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

private:
	Project m_Project;
	Ref<RuntimeSceneRenderer> m_SceneRenderer;

private:
	inline static App* s_Instance;
};

}
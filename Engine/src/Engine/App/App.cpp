#include "App.h"

#include <VolcaniCore/Core/Application.h>
#include <VolcaniCore/Core/Log.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/FileUtils.h>
#include <VolcaniCore/Window/Input.h>

#include <Engine/Graphics/Renderer.h>
#include <Engine/Graphics/Renderer2D.h>
#include <Engine/Graphics/Renderer3D.h>

#include <Engine/Script/ScriptModule.h>
#include <Engine/Script/ScriptClass.h>
#include <Engine/Script/ScriptObject.h>
#include <Engine/Physics/Physics.h>

#include <Engine/Asset/AssetManager.h>
#include <Engine/Scene/Component.h>

#include "ScriptGlue.h"

using namespace VolcanicEngine::Script;
using namespace VolcanicEngine::ECS;
// using namespace VolcanicEngine::Physics;

namespace fs = std::filesystem;

namespace VolcanicEngine {

static Ref<ScriptModule> s_AppModule = nullptr;
static Ref<ScriptObject> s_AppObject = nullptr;

static bool s_ScreenPrepared = false;
static bool s_ShouldSwitchScreen = false;
static bool s_ShouldPushScreen = false;
static bool s_ShouldPopScreen = false;
static std::string s_NewScreenName = "";

static Scene* s_ShouldLoadScene = nullptr;

struct RuntimeScreen {
	Ref<Scene> World;
	Ref<ScriptModule> Script;
	Ref<ScriptObject> ScriptObj;

	RuntimeScreen()
		: World(CreateRef<Scene>("")) { }

	~RuntimeScreen() {
		ScriptObj->Call("OnClose");
		ScriptObj.reset();
		Script.reset();

		App::Get()->GetSceneRenderer()->OnSceneClose();
		World->UnregisterSystems();
		World.reset();
	}
};

static RuntimeScreen* s_Screen = nullptr;

static Scene& ScriptGetScene() {
	return *s_Screen->World;
}

static asIScriptObject* GetScriptApp() {
	auto* handle = s_AppObject->GetHandle();
	handle->AddRef();
	return handle;
}

static AssetManager& GetAssetManagerInstance() {
	return *AssetManager::Get();
}

static void ScriptLoadScene(const std::string& name, App* app) {
	s_Screen->World->Name = name;
	s_Screen->World->World3D.Reset();
	s_Screen->World->World2D.Reset();
	s_Screen->World->Canvas.Reset();
	s_Screen->World->UnregisterSystems();
	s_Screen->World->RegisterSystems();
	app->GetSceneRenderer()->OnSceneLoad();
	app->SceneLoad(*s_Screen->World);

	List<Entity> list;
	s_Screen->World->World3D
	.ForEach<ScriptComponent>(
		[&](Entity entity)
		{
			auto& sc = entity.Set<ScriptComponent>();
			if(sc.Instance)
				list.Add(entity);
		});

	list.ForEach(
		[](Entity& entity)
		{
			auto& sc = entity.Set<ScriptComponent>();
			auto old = sc.Instance;
			if(!old->IsInitialized()) { // i.e Editor
				sc.Instance = old->GetClass()->Instantiate(entity);
				ScriptGlue::Copy(old, sc.Instance);
			}
			Log::Info("Loaded script for entity {}", entity.GetName());

			sc.Instance->Call("OnStart");
		});
}

static void AppLog(const std::string& msg, App* app) {
	app->Log(msg);
}

App::App() {
	s_Instance = this;

	ScriptEngine::RegisterSingleton("AppClass", "App", this);

	ScriptEngine::RegisterMethod<App>(
		"AppClass", "void SwitchScreen(const string &in)", &App::SwitchScreen);
	ScriptEngine::RegisterMethod<App>(
		"AppClass", "void PushScreen(const string &in)", &App::PushScreen);
	ScriptEngine::RegisterMethod<App>(
		"AppClass", "void PopScreen()", &App::PopScreen);
	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void Log(const string &in)", asFUNCTION(AppLog), asCALL_CDECL_OBJLAST);

	ScriptEngine::Get()->RegisterObjectMethod(
		"AppClass", "void LoadScene(const string &in)",
		asFUNCTION(ScriptLoadScene), asCALL_CDECL_OBJLAST);

	ScriptEngine::Get()->RegisterGlobalFunction(
		"SceneClass& get_Scene() property",
		asFUNCTION(ScriptGetScene), asCALL_CDECL);

	ScriptEngine::Get()->RegisterGlobalFunction(
		"IApp@ get_ScriptApp() property",
		asFUNCTION(GetScriptApp), asCALL_CDECL);
	ScriptEngine::Get()->RegisterGlobalFunction(
		"AssetManagerClass& get_AssetManager() property",
		asFUNCTION(GetAssetManagerInstance), asCALL_CDECL);
}

void App::CreateSceneRenderer() {
	m_SceneRenderer = CreateRef<RuntimeSceneRenderer>();
}

void App::OnLoad() {
	s_AppModule = CreateRef<ScriptModule>();
	AppLoad(s_AppModule);
	s_AppObject = s_AppModule->GetClass("Game")->Instantiate();
	s_AppObject->Call("OnLoad");
}

void App::OnClose() {
	delete s_Screen;
	s_Screen = nullptr;
	s_ScreenPrepared = false;

	if(s_AppObject)
		s_AppObject->Call("OnClose");

	s_AppObject.reset();
	s_AppModule.reset();
	m_SceneRenderer.reset();
}

void App::OnUpdate(TimeStep ts) {
	if(!Running)
		return;

	if(s_ShouldSwitchScreen)
		ScreenSet(s_NewScreenName);
	else if(s_ShouldPushScreen)
		ScreenPush(s_NewScreenName);
	else if(s_ShouldPopScreen)
		ScreenPop();

	s_AppObject->Call("OnUpdate", (f32)ts);

	if(!s_Screen)
		return;

	s_Screen->ScriptObj->Call("OnUpdate", (f32)ts);

	s_Screen->World->OnUpdate(ts);
	m_SceneRenderer->Update(ts);
	s_Screen->World->OnRender(*m_SceneRenderer);

	if(RenderScene) {
		auto output = m_SceneRenderer->GetOutput();
		// Renderer::StartPass(m_OutputPass);
		// {
		// 	Renderer2D::DrawFullscreenQuad(output);
		// }
		// Renderer::EndPass();
	}
}

void App::LoadScene(Scene* scene) {
	s_ShouldLoadScene = scene;
}

Scene* App::GetScene() {
	VOLCANICORE_ASSERT(s_Screen);
	return s_Screen->World.get();
}

void App::PrepareScreen() {
	s_ScreenPrepared = true;
	delete s_Screen;
	s_Screen = new RuntimeScreen();
}

void App::SwitchScreen(const std::string& name) {
	if(!ChangeScreen) {
		Running = false;
		return;
	}

	s_ShouldSwitchScreen = true;
	s_NewScreenName = name;
}

void App::PushScreen(const std::string& name) {
	if(!ChangeScreen) {
		Running = false;
		return;
	}

	s_ShouldPushScreen = true;
	s_NewScreenName = name;
}

void App::PopScreen(const std::string& name) {
	if(!ChangeScreen) {
		Running = false;
		return;
	}

	s_ShouldPopScreen = true;
	s_NewScreenName = name;
}

void App::ScreenSet(const std::string& name) {
	s_ShouldSwitchScreen = false;
	if(name == "")
		return;

	if(!s_ScreenPrepared)
		PrepareScreen();

	s_ScreenPrepared = false;

	ScreenLoad(s_Screen->Script, name);
	auto scriptClass = s_Screen->Script->GetClass(name);
	s_Screen->ScriptObj = scriptClass->Instantiate();

	if(s_ShouldLoadScene) {
		m_SceneRenderer->OnSceneLoad();
		s_Screen->World->RegisterSystems();
		*s_Screen->World = *s_ShouldLoadScene;
		s_ShouldLoadScene = nullptr;

		List<Entity> list;
		s_Screen->World->World3D
		.ForEach<ScriptComponent>(
			[&](Entity entity)
			{
				const auto& sc = entity.Get<ScriptComponent>();
				if(sc.Instance)
					list.Add(entity);
			});

		list.ForEach(
			[](Entity& entity)
			{
				auto& sc = entity.Set<ScriptComponent>();
				auto old = sc.Instance;
				sc.Instance = old->GetClass()->Instantiate(entity);
				ScriptGlue::Copy(old, sc.Instance);
				sc.Instance->Call("OnStart");
			});
	}
	// else if(screen.Scene != "")
	// 	ScriptLoadScene(screen.Scene, this);

	s_Screen->ScriptObj->Call("OnLoad");
}

void App::ScreenPush(const std::string& name) {

}

void App::ScreenPop() {

}

}

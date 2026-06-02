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

#include <Engine/Scene/Graphics/DefaultRenderPipeline.h>
#include <Engine/Scene/Graphics/ScriptRenderPipeline.h>

#include "ScriptGlue.h"

using namespace VolcanicEngine::Script;
using namespace VolcanicEngine::ECS;

namespace fs = std::filesystem;

namespace VolcanicEngine {

// ── Screen state ──────────────────────────────────────────────────────────────

static Ref<ScriptModule> s_AppModule;
static Ref<ScriptObject> s_AppObject;
static Ref<Scene> s_Scene;

// ── Script-glue helpers ───────────────────────────────────────────────────────

static Scene& ScriptGetScene() { return *s_Scene; }

static asIScriptObject* GetScriptApp() {
	if(!s_AppObject)
		return nullptr;
	auto* handle = s_AppObject->GetHandle();
	handle->AddRef();
	return handle;
}

static AssetManager& GetAssetManagerInstance() {
	return *AssetManager::Get();
}

static void ScriptLoadScene(const std::string& name, App* app) {
	auto& world = *s_Scene;
	world.Name = name;
	world.World3D.Reset();
	world.World2D.Reset();
	world.Canvas.Reset();
	world.UnregisterSystems();
	world.RegisterSystems();

	app->GetSceneRenderer().OnSceneLoad();
	app->SceneLoad(world);

	List<Entity> list;
	world.World3D.ForEach<ScriptComponent>(
		[&](Entity entity) {
			auto& sc = entity.Set<ScriptComponent>();
			if(sc.Instance)
				list.Add(entity);
		});

	list.ForEach(
		[](Entity& entity) {
			auto& sc = entity.Set<ScriptComponent>();
			auto old = sc.Instance;
			if(!old->IsInitialized()) {
				sc.Instance = old->GetClass()->Instantiate(entity);
				ScriptGlue::Copy(old, sc.Instance);
			}
			sc.Instance->Call("OnStart");
		});
}

static void AppLog(const std::string& msg, App* app) {
	app->Log(msg);
}

static void UseDefaultRenderPipeline(App* app) {
	app->UseDefaultPipeline();
}

static void UseDefaultRenderPipelineSized(
	int renderW, int renderH, App* app)
{
	app->UseDefaultPipeline((u32)renderW, (u32)renderH);
}

static void ScriptAddRenderHook(asIScriptObject* obj, App* app) {
	app->AddRenderHook(obj);
}

static void ScriptRemoveRenderHook(asIScriptObject* obj, App* app) {
	app->RemoveRenderHook(obj);
}

App::App() {
	s_Instance = this;

	ScriptEngine::RegisterSingleton("AppClass", "App", this);

	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void Log(const string &in)",
		asFUNCTION(AppLog), asCALL_CDECL_OBJLAST);
	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void LoadScene(const string &in)",
		asFUNCTION(ScriptLoadScene), asCALL_CDECL_OBJLAST);
	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void UseDefaultRenderPipeline()",
		asFUNCTION(UseDefaultRenderPipeline), asCALL_CDECL_OBJLAST);
	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void UseDefaultRenderPipeline(int renderW, int renderH)",
		asFUNCTION(UseDefaultRenderPipelineSized), asCALL_CDECL_OBJLAST);
	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void AddRenderHook(IRenderHook@)",
		asFUNCTION(ScriptAddRenderHook), asCALL_CDECL_OBJLAST);
	ScriptEngine::Get()->RegisterObjectMethod("AppClass",
		"void RemoveRenderHook(IRenderHook@)",
		asFUNCTION(ScriptRemoveRenderHook), asCALL_CDECL_OBJLAST);

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

App::~App() {
	s_Instance = nullptr;
}

// ── Pipeline API ──────────────────────────────────────────────────────────────

void App::UseDefaultPipeline() {
	m_SceneRenderer.UseDefaultPipeline();
}

void App::UseDefaultPipeline(u32 renderW, u32 renderH) {
	m_SceneRenderer.UseDefaultPipeline(renderW, renderH);
}

void App::AddRenderHook(asIScriptObject* obj) {
	auto* dp =
		dynamic_cast<DefaultRenderPipeline*>(
			m_SceneRenderer.GetPipeline().get());
	if(dp)
		dp->AddRenderHook(obj);
}

void App::RemoveRenderHook(asIScriptObject* obj) {
	auto* dp =
		dynamic_cast<DefaultRenderPipeline*>(
			m_SceneRenderer.GetPipeline().get());
	if(dp)
		dp->RemoveRenderHook(obj);
}

void App::ClearPipelineRenderHooks() {
	auto* dp =
		dynamic_cast<DefaultRenderPipeline*>(
			m_SceneRenderer.GetPipeline().get());
	if(dp)
		dp->ClearRenderHooks();
}

void App::SetPipeline(asIScriptObject* pipelineObj) {
	m_SceneRenderer.SetPipeline(
		CreateRef<ScriptRenderPipeline>(pipelineObj));
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void App::OnLoad() {
	Log::Info("App load start");
	if(s_AppModule)
		s_AppModule.reset();

	s_AppModule = AppLoad();

	if(!s_AppModule) {
		Log::Error("Failed to load app script module");
		return;
	}

	Log::Info("App module loaded");

	auto gameClass = s_AppModule->GetClass("Game");
	if(!gameClass) {
		Log::Error("Failed to find Game class in app module");
		return;
	}

	if(auto* mod = s_AppModule->GetHandle())
		mod->ResetGlobalVars();

	s_AppObject = gameClass->Instantiate();
	if(!s_AppObject) {
		Log::Error("Failed to instantiate Game");
		return;
	}

	s_AppObject->Call("OnLoad");
	Log::Info("App loaded");
}

void App::OnClose() {
	if(s_AppObject)
		s_AppObject->Call("OnClose");

	m_SceneRenderer.OnSceneClose();

	ScriptEngine::ResetContexts();

	s_Scene.reset();
	s_AppObject.reset();
	ScriptEngine::CollectGarbage();

	if(auto* mod = s_AppModule ? s_AppModule->GetHandle() : nullptr)
		mod->ResetGlobalVars();
	Log::Info("App closed");
}

void App::ReleaseScriptModule() {
	ScriptEngine::CollectGarbage();
	s_AppModule.reset();
}

void App::OnUpdate(TimeStep ts) {
	if(!Running || !s_AppObject)
		return;

	s_AppObject->Call("OnUpdate", (f32)ts);

	if(!s_Scene)
		return;

	s_Scene->OnUpdate(ts);
	m_SceneRenderer.Render(s_Scene.get(), ts);
}

void App::LoadScene(Scene* scene) {
	s_Scene = CreateRef<Scene>(scene->Name);
	SceneLoad(*s_Scene);
	Log::Info("Scene '{}' loaded", scene->Name);
}

Scene* App::GetScene() {
	VOLCANICORE_ASSERT(s_Scene);
	return s_Scene.get();
}

}
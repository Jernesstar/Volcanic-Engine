#pragma once

#define IM_ASSERT(x) ((void)0)
#define assert(x) ((void)0)
#include <imgui/imgui.h>

#include <Engine/Scene/Scene.h>
#include <Engine/Graphics/Platform/Framebuffer.h>

#include <Editor/App/EditorRenderPipeline.h>
#include "Panel.h"

using namespace VolcanicEngine;
using namespace VolcanicEngine::Graphics;

namespace VolcanicEditor {

class SceneHierarchyPanel : public Panel {
public:
	SceneHierarchyPanel(Scene* scene = nullptr)
		: Panel("Scene Hierarchy"), m_Scene(scene) { }

	void SetContext(Scene* scene) { m_Scene = scene; }
	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;

private:
	Scene* m_Scene = nullptr;
};

class SceneVisualizerPanel : public Panel {
public:
	enum class ViewMode { Composite, World3D, World2D, Canvas };

	SceneVisualizerPanel(Scene* scene = nullptr)
		: Panel("Scene Visualizer"), m_Scene(scene) { }

	void SetContext(Scene* scene);
	ViewMode GetViewMode() const { return m_ViewMode; }
	void OnResize(u32 w, u32 h);
	Ref<Framebuffer> GetActiveFramebuffer() const;

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;

private:
	void DrawViewport();

	Scene* m_Scene = nullptr;
	ViewMode m_ViewMode = ViewMode::Composite;
	EditorRenderPipeline m_EditorPipeline;
	ImVec2 m_LastSize;
};

class ComponentEditorPanel : public Panel {
public:
	ComponentEditorPanel()
		: Panel("Component Editor") { }

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;

private:
	ECS::Entity m_Context;
};

class ContentBrowserPanel : public Panel {
public:
	ContentBrowserPanel()
		: Panel("Content Browser") { }

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;
};

class AssetEditorPanel : public Panel {
public:
	AssetEditorPanel()
		: Panel("Asset Editor") { }

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;
};

class ConsolePanel : public Panel {
public:
	ConsolePanel()
		: Panel("Console") { }

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;
};

}
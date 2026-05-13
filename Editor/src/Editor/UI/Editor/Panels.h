#pragma once

#define IM_ASSERT(x) ((void)0)
#define assert(x) ((void)0)
#include <imgui/imgui.h>

#include <Engine/Scene/Scene.h>

#include "Panel.h"

using namespace VolcanicEngine;

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

	void SetContext(Scene* scene) { m_Scene = scene; }
	ViewMode GetViewMode() const { return m_ViewMode; }

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;

private:
	Scene* m_Scene = nullptr;
	ViewMode m_ViewMode = ViewMode::Composite;
};

class ComponentEditorPanel : public Panel {
public:
	ComponentEditorPanel()
		: Panel("Component Editor") { }

	void Update(VolcaniCore::TimeStep ts) override;
	void Draw() override;
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
#pragma once

#include <imgui/imgui.h>

#include <Engine/Scene/Scene.h>

#include "Panel.h"

using namespace VolcanicEngine;

namespace VolcanicEditor {

// ─────────────────────────────────────────────
// SceneHierarchyPanel
// Three collapsible vertical sections:
//   World3D / World2D / Canvas
// ─────────────────────────────────────────────
class SceneHierarchyPanel : public Panel {
public:
	SceneHierarchyPanel(Scene* scene = nullptr)
		: Panel("Scene Hierarchy"), m_Scene(scene) { }

	void SetContext(Scene* scene) { m_Scene = scene; }

	void Draw() override {
		ImGui::Begin("Scene Hierarchy", &Open);

		if(ImGui::CollapsingHeader("World 3D", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent();
			if(m_Scene) {
				// TODO: iterate m_Scene->World3D entities
				ImGui::TextDisabled("(no entities)");
			}
			else
				ImGui::TextDisabled("(no scene)");
			ImGui::Unindent();
		}

		if(ImGui::CollapsingHeader("World 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent();
			if(m_Scene)
				ImGui::TextDisabled("(no entities)");
			else
				ImGui::TextDisabled("(no scene)");
			ImGui::Unindent();
		}

		if(ImGui::CollapsingHeader("Canvas", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent();
			if(m_Scene)
				ImGui::TextDisabled("(no entities)");
			else
				ImGui::TextDisabled("(no scene)");
			ImGui::Unindent();
		}

		ImGui::End();
	}

private:
	Scene* m_Scene = nullptr;
};

// ─────────────────────────────────────────────
// SceneVisualizerPanel
// Tab bar at top: 3D | 2D | Canvas | Composite
// ─────────────────────────────────────────────
class SceneVisualizerPanel : public Panel {
public:
	enum class ViewMode { World3D, World2D, Canvas, Composite };

	SceneVisualizerPanel(Scene* scene = nullptr)
		: Panel("Scene Visualizer"), m_Scene(scene) { }

	void SetContext(Scene* scene) { m_Scene = scene; }
	ViewMode GetViewMode() const { return m_ViewMode; }

	void Draw() override {
		auto flags = ImGuiWindowFlags_NoScrollbar
				   | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::Begin("Scene Visualizer", &Open, flags);
		ImGui::PopStyleVar();

		if(ImGui::BeginTabBar("VisualizerTabs")) {
			if(ImGui::BeginTabItem("3D")) {
				m_ViewMode = ViewMode::World3D;
				DrawViewport("3D viewport");
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("2D")) {
				m_ViewMode = ViewMode::World2D;
				DrawViewport("2D viewport");
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Canvas")) {
				m_ViewMode = ViewMode::Canvas;
				DrawViewport("Canvas viewport");
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Composite")) {
				m_ViewMode = ViewMode::Composite;
				DrawViewport("Composite viewport");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

private:
	Scene* m_Scene = nullptr;
	ViewMode m_ViewMode = ViewMode::World3D;

	void DrawViewport(const char* label) {
		ImVec2 size = ImGui::GetContentRegionAvail();
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.08f, 0.08f, 0.08f, 1.0f });
		ImGui::BeginChild(label, size);
		{
			// TODO: blit framebuffer texture here
			ImVec2 center = {
				ImGui::GetCursorPosX() + size.x * 0.5f - 50.0f,
				ImGui::GetCursorPosY() + size.y * 0.5f - 7.0f
			};
			ImGui::SetCursorPos(center);
			ImGui::TextDisabled("[%s]", label);
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
};

// ─────────────────────────────────────────────
// ComponentEditorPanel
// ─────────────────────────────────────────────
class ComponentEditorPanel : public Panel {
public:
	ComponentEditorPanel()
		: Panel("Component Editor") { }

	void Draw() override {
		ImGui::Begin("Component Editor", &Open);
		ImGui::TextDisabled("(no entity selected)");
		ImGui::End();
	}
};

// ─────────────────────────────────────────────
// ContentBrowserPanel
// ─────────────────────────────────────────────
class ContentBrowserPanel : public Panel {
public:
	ContentBrowserPanel()
		: Panel("Content Browser") { }

	void Draw() override {
		ImGui::Begin("Content Browser", &Open);

		ImVec2 avail = ImGui::GetContentRegionAvail();

		// Left: file hierarchy stub
		ImGui::BeginChild("CB_FileHierarchy",
			{ avail.x * 0.3f, avail.y }, ImGuiChildFlags_Borders);
		ImGui::SeparatorText("Files");
		ImGui::TextDisabled("(project files)");
		ImGui::EndChild();

		ImGui::SameLine();

		// Right: asset grid stub
		ImGui::BeginChild("CB_Assets", { }, ImGuiChildFlags_Borders);
		ImGui::SeparatorText("Assets");
		ImGui::TextDisabled("(asset thumbnails)");
		ImGui::EndChild();

		ImGui::End();
	}
};

// ─────────────────────────────────────────────
// AssetEditorPanel
// ─────────────────────────────────────────────
class AssetEditorPanel : public Panel {
public:
	AssetEditorPanel()
		: Panel("Asset Editor") { }

	void Draw() override {
		ImGui::Begin("Asset Editor", &Open);
		ImGui::TextDisabled("(no asset selected)");
		ImGui::End();
	}
};

// ─────────────────────────────────────────────
// ConsolePanel
// ─────────────────────────────────────────────
class ConsolePanel : public Panel {
public:
	ConsolePanel()
		: Panel("Console") { }

	void Draw() override {
		ImGui::Begin("Console", &Open);

		// Log area
		ImVec2 logSize = ImGui::GetContentRegionAvail();
		logSize.y -= ImGui::GetFrameHeightWithSpacing() + 4.0f;
		ImGui::BeginChild("ConsoleLog", logSize, ImGuiChildFlags_Borders);
		ImGui::TextDisabled("(log output)");
		ImGui::EndChild();

		// Tab bar at bottom
		if(ImGui::BeginTabBar("ConsoleTabs",
			ImGuiTabBarFlags_NoTabListScrollingButtons))
		{
			if(ImGui::BeginTabItem("Log")) ImGui::EndTabItem();
			if(ImGui::BeginTabItem("Debug")) ImGui::EndTabItem();
			ImGui::EndTabBar();
		}

		ImGui::End();
	}
};

}
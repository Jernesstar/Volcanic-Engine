#include "Panels.h"

namespace VolcanicEditor {

static void DrawViewport(const char* label) {
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

void SceneVisualizerPanel::Update(VolcaniCore::TimeStep ts) {

}

void SceneVisualizerPanel::Draw() {
	auto flags = ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::Begin("Scene Visualizer", &Open, flags);
	ImGui::PopStyleVar();

	if(ImGui::BeginTabBar("VisualizerTabs")) {
		if(ImGui::BeginTabItem("Composite")) {
			m_ViewMode = ViewMode::Composite;
			DrawViewport("Composite viewport");
			ImGui::EndTabItem();
		}
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
		ImGui::EndTabBar();
	}

	ImGui::End();
}

}
#include "Panels.h"

namespace VolcanicEditor {

void ConsolePanel::Update(VolcaniCore::TimeStep ts) {

}

void ConsolePanel::Draw()  {
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

}
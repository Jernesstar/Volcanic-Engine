#include "Panels.h"

namespace VolcanicEditor {

void ContentBrowserPanel::Update(VolcaniCore::TimeStep ts) {

}

void ContentBrowserPanel::Draw() {
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

}
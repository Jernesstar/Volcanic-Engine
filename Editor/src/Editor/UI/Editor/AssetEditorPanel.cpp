#include "Panels.h"

namespace VolcanicEditor {

void AssetEditorPanel::Update(VolcaniCore::TimeStep ts) {

}

void AssetEditorPanel::Draw() {
	ImGui::Begin("Asset Editor", &Open);
	ImGui::TextDisabled("(no asset selected)");
	ImGui::End();
}

}
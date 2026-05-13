#include "Panels.h"

namespace VolcanicEditor {

void ComponentEditorPanel::Update(VolcaniCore::TimeStep ts) {

}

void ComponentEditorPanel::Draw() {
	ImGui::Begin("Component Editor", &Open);
	ImGui::TextDisabled("(no entity selected)");
	ImGui::End();
}

}
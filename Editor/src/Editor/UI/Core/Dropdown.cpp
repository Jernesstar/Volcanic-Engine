#include "Dropdown.h"

#include <imgui/imgui.h>

#include "UIRenderer.h"

using namespace VolcaniCore;

namespace VolcanicEditor {

void Dropdown::Draw() {
	m_State = UIRenderer::DrawDropdown(*this);
}

}
#include "Text.h"

#include "UIRenderer.h"

namespace VolcanicEditor {

void Text::Draw() {
	m_State = UIRenderer::DrawText(*this);
	
}

}
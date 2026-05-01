#include "TextInput.h"

#include "UIRenderer.h"

namespace VolcanicEditor {

void TextInput::Draw() {
	m_State = UIRenderer::DrawTextInput(*this);
}

}
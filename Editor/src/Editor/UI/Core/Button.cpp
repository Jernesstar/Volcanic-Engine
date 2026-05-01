#include "Button.h"

#include "UIRenderer.h"

#include "Text.h"
#include "Image.h"

namespace VolcanicEditor {

void Button::Draw() {
	m_State = UIRenderer::DrawButton(*this);
}

}
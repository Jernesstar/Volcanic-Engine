#include "Window.h"

#include "UIRenderer.h"

namespace VolcanicEditor {

void Window::Draw() {
	m_State = UIRenderer::DrawWindow(*this);
}

}
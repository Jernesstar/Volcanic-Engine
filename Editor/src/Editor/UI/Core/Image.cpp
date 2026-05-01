#include "Image.h"

#include "UIRenderer.h"

using namespace VolcaniCore;

namespace VolcanicEditor {

void Image::Draw() {
	m_State = UIRenderer::DrawImage(*this);
}

}
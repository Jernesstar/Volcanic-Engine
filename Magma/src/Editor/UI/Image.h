#pragma once

#include <Magma/Graphics/Framebuffer.h>

#include "UIElement.h"

namespace Magma::UI {

class Image : public UIElement {
public:
	Ref<Graphics::Texture> Content;
	uint64_t ImageID = 0;

public:
	Image()
		: UIElement(UIElementType::Image) { }
	Image(Ref<Graphics::Texture> image)
		: UIElement(UIElementType::Image), Content(image) { }

	void Draw() override;
};

}
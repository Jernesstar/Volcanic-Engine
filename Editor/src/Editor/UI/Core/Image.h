#pragma once

#include <Engine/Graphics/Platform/Texture.h>

#include "UIElement.h"

using namespace VolcanicEngine;

namespace VolcanicEditor {

class Image : public UIElement {
public:
	Ref<Graphics::Texture> Content;
	uint64_t ImageID = 0;

public:
	Image()
		: UIElement(UIElementType::Image) { }
	Image(const std::string& id, UIPage* root)
		: UIElement(UIElementType::Image, id, root) { }
	Image(Ref<Graphics::Texture> image)
		: UIElement(UIElementType::Image), Content(image) { }

	void Draw() override;
};

}
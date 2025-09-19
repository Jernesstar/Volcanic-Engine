#pragma once

#include "UIElement.h"

#include "Editor/AssetImporter.h"

namespace Magma::UI {

class Image : public UIElement {
public:
	Ref<Texture> Content;

public:
	Image()
		: UIElement(UIElementType::Image) { }
	Image(Ref<Texture> image)
		: UIElement(UIElementType::Image), Content(std::move(image)) { }

	void Draw() override;
};

}
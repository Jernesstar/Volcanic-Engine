#pragma once

#include "UIElement.h"

#include "Image.h"
#include "Text.h"

using namespace VolcaniCore;

namespace VolcanicEditor {

class Button : public UIElement {
public:
	Ref<UIElement> Display;

public:
	Button()
		: UIElement(UIElementType::Button) { }
	Button(const std::string& id, UIPage* root)
		: UIElement(UIElementType::Button, id, root) { }

	void Draw() override;
};

}
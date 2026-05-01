#pragma once

#include "UIElement.h"

namespace VolcanicEditor {

class TextInput : public UIElement {
public:
	std::string Text;
	std::string Hint;
	uint32_t MaxCharCount;

public:
	TextInput(const std::string& id, UIPage* root)
		: UIElement(UIElementType::TextInput, id, root), MaxCharCount(100) { }

	void Draw() override;
};

}
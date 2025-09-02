#pragma once

#include "UIElement.h"

namespace Magma::UI {

class TextInput : public UIElement {
public:
	std::string Text;
	std::string Hint;
	uint32_t MaxCharCount;

public:
	TextInput(const std::string& id)
		: UIElement(UIElementType::TextInput), MaxCharCount(100) { }

	void Draw() override;
};

}
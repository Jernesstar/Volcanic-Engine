#include "UIElement.h"

#include <VolcaniCore/Core/Log.h>

using namespace VolcaniCore;

namespace VolcanicEditor {

UINode UIElement::Add(UIElementType type, const std::string& id) {
	return { type, 0 };
}

void UIElement::Add(const UINode& node) {
	Children.Add(node);
}

void UIElement::Render() {
	Draw();
}

UIElement& UIElement::SetSize(uint32_t width, uint32_t height) {
	this->Width = width;
	this->Height = height;
	return *this;
}
UIElement& UIElement::SetPosition(float x, float y) {
	this->x = x;
	this->y = y;
	return *this;
}

UIElement& UIElement::CenterX() {
	return *this;
}

UIElement& UIElement::CenterY() {
	return *this;
}

UIElement& UIElement::Center() {
	CenterX();
	CenterY();
	return *this;
}

void UIElement::Clear() {
	Children.Clear();
}

UIElement* UIElement::GetParent() const {
	return nullptr;
}

UIElement* UIElement::GetChild(const UINode& node) const {
	return nullptr;
}

UIElement* UIElement::GetChild(const std::string& id) const {
	return nullptr;
}

List<UIElement*> UIElement::GetChildren() const {
	List<UIElement*> res;
	for(auto node : Children)
		res.Add(GetChild(node));

	return res;
}

}
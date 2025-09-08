#pragma once

#include <glm/vec4.hpp>

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/UUID.h>

namespace Magma::UI {

class UIPage;

struct UIState {
	bool Clicked = false;
	bool Hovered = false;
	bool MouseUp = false;
	bool MouseDown = false;
};

enum class XAlignment { Left, Center, Right };
enum class YAlignment { Top, Center, Bottom };

enum class UIElementType {
	Empty,
	Window,
	Button,
	Dropdown,
	Text,
	TextInput,
	Image,
	None
};

using UINode = std::pair<UIElementType, uint32_t>;

class UIElement : public VolcaniCore::Derivable<UIElement> {
public:
	uint32_t Width = 0;
	uint32_t Height = 0;
	float x = 0;
	float y = 0;
	XAlignment xAlignment = XAlignment::Left;
	YAlignment yAlignment = YAlignment::Top;
	bool UsePosition = true;
	glm::vec4 Color = glm::vec4(0.0f);

public:
	UIElement(UIElementType type)
		: m_Type(type) { }
	virtual ~UIElement() = default;

	virtual void Draw() = 0;

	UIState GetState() const { return m_State; }

	UIElement& SetSize(uint32_t width, uint32_t height) {
		this->Width = width;
		this->Height = height;
		return *this;
	}
	UIElement& SetPosition(float x, float y) {
		this->x = x;
		this->y = y;
		return *this;
	}

	bool Is(UIElementType type) const { return m_Type == type; }
	UIElementType GetType() const { return m_Type; }

protected:
	const UIElementType m_Type;
	UIState m_State;

	friend class UIPage;
};

}
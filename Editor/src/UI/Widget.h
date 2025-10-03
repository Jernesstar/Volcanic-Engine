#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/TimeUtils.h>

#include <Magma/Script/ScriptEngine.h>

using namespace VolcaniCore;
using namespace Magma::Script;

namespace Magma::UI {

class Animation {
public:
	Animation() = default;
	virtual ~Animation() = default;
	virtual void Update(TimeStep ts) = 0;
};

enum class WidgetType {
	None,
	Window,
	Container,
	Panel,
	Button,
	Dropdown,
	Text,
	TextInput,
	Image,
	FileDialog,
	FileEditor,
};

struct UIState {
	bool Clicked = false;
	bool Held = false;
	bool Released = false;
	bool Hovered = false;
	bool NavFocused = false;
	bool Dragging = false;
};

class Widget {
public:
	const WidgetType Type;
	std::string ID;
	UIState State;
	Ref<Widget> Parent = nullptr;
	List<Ref<Widget>> Children;
	List<Ref<Animation>> Animations;

	bool Visible = true;
	bool Enabled = true;

	uint32_t Width = 0, Height = 0;
	float x = 0, y = 0;

public:
	Widget(const std::string& id, WidgetType type)
		: ID(id), Type(type) { }
	virtual ~Widget() = default;

	virtual void Begin() = 0;
	virtual void End() = 0;

	void Update(TimeStep ts) {
		if(!Enabled)
			return;

		for(auto& animations : Animations)
			animations->Update(ts);
		for(auto& widget : Children)
			widget->Update(ts);
	}

	void Render() {
		if(!Visible)
			return;

		Begin();
		for(auto& widget : Children)
			widget->Render();
		End();
	}

	bool Is(WidgetType type) const { return Type == type; }
};

class Window : Widget {
public:
	enum class Option { MenuBar, TitleBar };
	List<Option> Options;

public:
	Window(const std::string& id)
		: { }
	Window* With(Window::Option option);
};

class Container : Widget {
public:
	Container(const std::string& id)
		: Widget(id, WidgetType::Container) { }
};

class WidgetRenderer {
public:
	static void RegisterInterface();
	static void BeginFrame();
	static void EndFrame();

	static Window* Window(const std::string& name);
	static Container* Panel();
	static TextWidget* Text();
	static ImageWidget* Image();
};

}
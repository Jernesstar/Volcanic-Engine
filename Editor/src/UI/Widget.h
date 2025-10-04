#pragma once

#define IM_VEC2_CLASS_EXTRA \
	constexpr ImVec2(glm::vec2& v) : x(v.x), y(v.y) {} \
	operator glm::vec2() const { return glm::vec2(x, y); }

#define IM_VEC3_CLASS_EXTRA \
	constexpr ImVec3(glm::vec3& v) : x(v.x), y(v.y), z(v.z) {} \
	operator glm::vec3() const { return glm::vec3(x, y, z); }

#define IM_VEC4_CLASS_EXTRA \
	constexpr ImVec4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {} \
	operator glm::vec4() const { return glm::vec4(x, y, z, w); }

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ImGuiColorTextEdit/TextEditor.h>

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <Magma/Script/ScriptEngine.h>

#include "Editor/AssetImporter.h"

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
	Dropdown,
	Button,
	Image,
	Text,
	TextInput,
	Gizmo,
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
	const std::string ID;
	const WidgetType Type;

	Ref<Widget> Parent = nullptr;
	List<Ref<Widget>> Children;
	List<Ref<Animation>> Animations;

	float Width = 0, Height = 0;
	float x = 0, y = 0;
	bool Visible = true;
	bool Enabled = true;

	bool StateChange = false;
	UIState State;

	bool IsNative = true;
	ScriptFunc OnEventScript;
	Func<void, const UIState&> OnEvent;

public:
	Widget(const std::string& id, WidgetType type)
		: ID(id), Type(type) { }
	virtual ~Widget() = default;

	virtual void Begin() = 0;
	virtual void End() = 0;

	void Update(TimeStep ts) {
		if(!Enabled)
			return;

		if(StateChange) {
			if(IsNative)
				OnEvent(State);
			else
				OnEventScript.CallVoid(State);

			StateChange = false;
		}

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

class Window : public Widget {
public:
	bool MenuBar = false;
	bool IsChild = false;

	Vec4 Color;

public:
	Window(const std::string& id)
		: Widget(id, WidgetType::Window) { }

	void Begin() override;
	void End() override;
};

class Container : public Widget {
public:
	Container(const std::string& id)
		: Widget(id, WidgetType::Container) { }
};

class Dropdown : public Widget {
public:
	uint64_t CurrentItem = 0;
	VolcaniCore::List<std::string> Options;

public:
	Dropdown(const std::string& id)
		: Widget(id, WidgetType::Dropdown) { }
};

class Button : public Widget {
public:
	Button(const std::string& id)
		: Widget(id, WidgetType::Button) { }
};

class Image : public Widget {
public:
	Ref<Magma::Texture> Content;

public:
	Image(const std::string& id)
		: Widget(id, WidgetType::Image) { }
};

class Text : public Widget {
public:
	std::string Label;

public:
	Text(const std::string& id)
		: Widget(id, WidgetType::Text) { }
};

class TextInput : public Widget {
public:
	std::string Text;
	std::string Hint;
	uint32_t MaxCharCount;

public:
	TextInput(const std::string& id)
		: Widget(id, WidgetType::TextInput) { }
};

class Gizmo : public Widget {
public:
	Gizmo(const std::string& id)
		: Widget(id, WidgetType::Gizmo) { }
};

class FileDialog : public Widget {
public:
	std::string Title;
	std::string StartPath;
	bool OpenDir = false;
	List<std::string> Extensions;
	Func<void, const fs::path&> OnSelect;

public:
	FileDialog(const std::string& id)
		: Widget(id, WidgetType::FileDialog) { }
};

class FileEditor : public Widget {
public:
	TextEditor Editor;

public:
	FileEditor(const std::string& id)
		: Widget(id, WidgetType::FileEditor) { }
};

class WidgetManager {
public:
	static void RegisterInterface();
	static void BeginFrame();
	static void EndFrame();

	static Window* Window(const std::string& name);
	static Container* Container(const std::string& name);
	static Dropdown* Dropdown(const std::string& name);
	static Text* Text(const std::string& name);
	static Image* Image(const std::string& name);
};

}
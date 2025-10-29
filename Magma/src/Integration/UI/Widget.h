#pragma once

#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/Defines.h>

using namespace VolcaniCore;

#define IM_VEC2_CLASS_EXTRA \
	constexpr ImVec2(Vec2& v) : x(v.x), y(v.y) {} \
	operator Vec2() const { return Vec2(x, y); }

#define IM_VEC3_CLASS_EXTRA \
	constexpr ImVec3(Vec3& v) : x(v.x), y(v.y), z(v.z) {} \
	operator Vec3() const { return Vec3(x, y, z); }

#define IM_VEC4_CLASS_EXTRA \
	constexpr ImVec4(const Vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {} \
	operator Vec4() const { return Vec4(x, y, z, w); }

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ImGuiColorTextEdit/TextEditor.h>

#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <Magma/Script/ScriptEngine.h>

#include "Core/Integration.h"
#include "Core/AssetImporter.h"

using namespace Magma::Script;

namespace Magma::UI {

enum class WidgetType {
	None,
	Root,
	Window,
	Container,
	Dropdown,
	Button,
	Image,
	Text,
	TextInput,
	FileDialog,
	FileEditor,
	Gizmo,
};

struct UIState {
	bool Clicked = false;
	bool Held = false;
	bool Released = false;
	bool Hovered = false;
	bool NavFocused = false;
	bool Dragging = false;

	operator bool() const {
		return Clicked || Held || Released || Hovered || NavFocused || Dragging;
	}
};

class Widget;

class Animation {
public:
	Animation(float time, float duration)
		: m_Time(time), m_Duration(duration) { }
	virtual ~Animation() = default;

	virtual void Update(Widget* widget, TimeStep ts) = 0;

	void Reset() { m_Time = 0.0f; }

private:
	float m_Time = 0.0f;
	float m_Duration = 0.0f;
};

class FadeIn : public Animation {
	
};

class FadeOut : public Animation {
	
};

class Scale : public Animation {
	
};

class Translate : public Animation {
	
};

class Widget : public Derivable<Widget> {
public:
	const std::string ID;
	const WidgetType Type;

	Widget* Parent = nullptr;
	List<Ref<Widget>> Children;
	List<Ref<Animation>> Animations;

	float Width = 5, Height = 5;
	float x = 0, y = 0;
	bool Visible = true;
	bool Enabled = true;

	UIState State;

	bool IsNative = true;
	ScriptFunc OnEventScript = { };
	Func<void, const UIState&> OnEvent = [](const UIState&) { };

public:
	Widget(const std::string& id, WidgetType type)
		: ID(id), Type(type) { }
	Widget(Widget&&) = default;
	Widget(const Widget&) = default;
	virtual ~Widget() = default;

	virtual void Begin() = 0;
	virtual void End() = 0;

	void Update(TimeStep ts);
	void Render();

	bool Is(WidgetType type) const { return Type == type; }

	Widget* Add(Ref<Widget> widget);

	template<typename TWidget>
	requires std::derived_from<TWidget, Widget>
	Widget* AddCopy(Ref<TWidget> widget) {
		auto& ptr = Children.Emplace(CreateRef<Widget>(*widget));
		ptr->Parent = this;
		return this;
	}

	Ref<Widget> Find(const std::string& id);
	void Remove(const std::string& id);

private:
	void Reposition();
};

class Root : public Widget {
public:
	Root(const std::string& id)
		: Widget(id, WidgetType::Root) { }

	void Begin() override;
	void End() override;
};

class Window : public Widget {
public:
	bool MenuBar = false;
	bool IsRoot = false;
	bool IsChild = false;
	bool AllowResize = false;
	bool AllowMove = false;
	bool AllowDock = false;
	bool AllowClose = false;
	bool AllowScroll = false;
	bool AllowMinimize = false;
	bool AllowMaximize = false;

	Vec4 Color = { };

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

	void Begin() override;
	void End() override;
};

class Dropdown : public Widget {
public:
	uint64_t CurrentItem = 0;
	VolcaniCore::List<std::string> Options;

public:
	Dropdown(const std::string& id)
		: Widget(id, WidgetType::Dropdown) { }

	void Begin() override;
	void End() override;
};

class Button : public Widget {
public:
	Vec4 Color;

public:
	Button(const std::string& id)
		: Widget(id, WidgetType::Button) { }

	void Begin() override;
	void End() override;
};

class Image : public Widget {
public:
	Ref<Magma::Texture> Asset;

public:
	Image(const std::string& id)
		: Widget(id, WidgetType::Image) { }

	void Begin() override;
	void End() override;
};

class Text : public Widget {
public:
	std::string Label;
	float Scale = 1.0f;

public:
	Text(const std::string& id)
		: Widget(id, WidgetType::Text) { }

	void Begin() override;
	void End() override;
};

class TextInput : public Widget {
public:
	std::string Text;
	std::string Hint;
	uint32_t MaxCharCount;

public:
	TextInput(const std::string& id)
		: Widget(id, WidgetType::TextInput) { }

	void Begin() override;
	void End() override;
};

class FileDialog : public Widget {
public:
	std::string Title;
	std::string StartPath;
	bool OpenDir = false;
	List<std::string> Extensions;
	Func<void, const fs::path&> OnSelect;

	bool StartSelect = false;

private:
	IGFD::FileDialogConfig Config;

public:
	FileDialog(const std::string& id)
		: Widget(id, WidgetType::FileDialog) { }

	void Begin() override;
	void End() override;
};

class FileEditor : public Widget {
public:
	TextEditor Editor;

public:
	FileEditor(const std::string& id)
		: Widget(id, WidgetType::FileEditor) { }

	void Begin() override;
	void End() override;
};

class Gizmo : public Widget {
public:
	enum class GizmoMode {
		Translate,
		Rotate,
		Scale
	} Mode = GizmoMode::Translate;

public:
	Gizmo(const std::string& id)
		: Widget(id, WidgetType::Gizmo) { }

	void Begin() override;
	void End() override;
};

class UIManager : public Integration {
public:
	UIManager() = default;
	~UIManager() = default;

	void Init() override;
	void Shutdown() override;
	
	void BeginFrame();
	void EndFrame();
	void Update(TimeStep ts);
	void Render();

	void Load(const std::string& path);
	void Reload();
	void Clear();

	Ref<Widget> GetRoot() { return m_Root; }

	// static Window* Window(const std::string& name);
	// static Container* Container(const std::string& name);
	// static Dropdown* Dropdown(const std::string& name);
	// static Text* Text(const std::string& name);
	// static Image* Image(const std::string& name);

private:
	Ref<Widget> m_Root;
	std::string m_Path;
};

}
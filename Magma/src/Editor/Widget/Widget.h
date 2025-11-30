#pragma once

#include <VolcaniCore/Core/Defines.h>
#include <VolcaniCore/Core/Template.h>
#include <VolcaniCore/Core/Math.h>
#include <VolcaniCore/Core/List.h>
#include <VolcaniCore/Core/TimeUtils.h>
#include <VolcaniCore/Core/FileUtils.h>

#include <Lava/Script/ScriptEngine.h>

#include "Asset/Asset.h"
#include "Graphics/Platform/Texture.h"

using namespace VolcaniCore;
using namespace Lava::Script;

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
	ScriptFunc OnEvent = { };

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

	template<typename TWidget, typename ...Args>
	requires std::derived_from<TWidget, Widget>
	Widget* Add(Args&&... args) {
		auto ptr =
			Children.Emplace(
				CreateRef<TWidget>(std::forward<Args>(args)...));
		ptr->Parent = this;
		return this;
	}


	template<typename TWidget>
	requires std::derived_from<TWidget, Widget>
	Widget* AddCopy(Ref<TWidget> widget) {
		auto ptr = Children.Emplace(CreateRef<Widget>(*widget));
		ptr->Parent = this;
		return this;
	}

	Ref<Widget> Find(const std::string& id);
	void Remove(const std::string& id);
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

	Vec4 Color = { 1.0f, 0.0f, 0.0f, 1.0f };

public:
	Window(const std::string& id)
		: Widget(id, WidgetType::Window) { }

	void Begin() override;
	void End() override;
};

class Container : public Widget {
public:
	enum class LayoutType {
		Horizontal,
		Vertical
	} Layout = LayoutType::Horizontal;

	enum class SizeType {
		Fixed,
		Stretch
	};

	SizeType SizeX = SizeType::Stretch;
	SizeType SizeY = SizeType::Fixed;

	Vec4 Color = { 0.0f, 1.0f, 0.0f, 1.0f };

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
	Vec4 Color = { };

public:
	Button(const std::string& id)
		: Widget(id, WidgetType::Button) { }

	void Begin() override;
	void End() override;
};

class Image : public Widget {
public:
	AssetID Texture;

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
	Vec4 Color = { 0.0f, 0.0f, 1.0f, 1.0f };
	AssetID Font;

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

public:
	FileDialog(const std::string& id)
		: Widget(id, WidgetType::FileDialog) { }

	void Begin() override;
	void End() override;
};

class FileEditor : public Widget {
public:

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

class WidgetManager {
public:
	static void Init();
	static void Close();

	static void Update(TimeStep ts);
	static void Render();

	static void Load(const std::string& path);

	static Ref<Widget> AddRoot(const std::string& name) {
		auto ptr = m_Roots.Emplace(CreateRef<Root>(name));
		return ptr;
	}

	static void SetRoot(const std::string& name) {
		auto [found, i] = m_Roots.Find(
			[&](const auto& widget) { return widget->ID == name; });
		if(found)
			m_CurrentRoot = i + 1;
	}

	static Ref<Widget> GetRoot() {
		if(m_CurrentRoot)
			return m_Roots[m_CurrentRoot - 1];
		return nullptr;
	}

	static Ref<Widget> GetRoot(const std::string& name) {
		auto [found, i] = m_Roots.Find(
			[&](const auto& widget) { return widget->ID == name; });
		if(found)
			return m_Roots[i];
		return nullptr;
	}

	// static Window* Window(const std::string& name);
	// static Container* Container(const std::string& name);
	// static Dropdown* Dropdown(const std::string& name);
	// static Text* Text(const std::string& name);
	// static Image* Image(const std::string& name);

private:
	inline static List<Ref<Widget>> m_Roots;
	inline static u32 m_CurrentRoot = 0;
};

}
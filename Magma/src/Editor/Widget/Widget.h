#pragma once

#include <RmlUi/Core.h>

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

class WidgetManager {
public:
	static void Init();
	static void Close();

	static void Update(TimeStep ts);
	static void Render();

	static void Load(const std::string& path);
	static void Reload();
	static Rml::ElementDocument* GetDocument();

private:
	inline static std::string m_RootPath = "";
};

class ElementEventListener : public Rml::EventListener {
public:
	ElementEventListener(Rml::Element* element,
		Func<void, Rml::Element*, Rml::Event&> callback)
		: m_Element(element), m_Callback(callback) { }

	void ProcessEvent(Rml::Event& event) override {
		if(m_Element->GetId() == event.GetTargetElement()->GetId())
			m_Callback(m_Element, event);
	}

	void OnDetach(Rml::Element* element) override {
		delete this;
	}

private:
	Rml::Element* m_Element;
	Func<void, Rml::Element*, Rml::Event&> m_Callback;
};

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

}